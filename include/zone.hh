#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <boost/unordered_map.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "common_types.hh"
#include "bounds.hh"
#include "constraint.hh"

#include <Eigen/Core>
#include <utility>

namespace learnta {

  /*!
    @brief Implementation of a zone with DBM

    @note internally, the variable 0 is used for the constant while externally, the actual clock variable is 0 origin, i.e., the variable 0 for the user is the variable 1 internally. So, we need increment or decrement to fill the gap.
  */
  struct Zone {
    //! @brief The matrix representing the DBM
    Eigen::Matrix <Bounds, Eigen::Dynamic, Eigen::Dynamic> value;
    //! @brief The threshold for the normalization
    Bounds M;
    //! @brief the threshold of each clock variable
    std::vector<double> maxConstraints;

    Zone() = default;

    //! @brief Construct a zone from a matrix representing the zone
    explicit Zone(const Eigen::Matrix <Bounds, Eigen::Dynamic, Eigen::Dynamic> &value) :
            Zone(Eigen::Matrix<Bounds, Eigen::Dynamic, Eigen::Dynamic>(value)) {}

    //! @brief Construct a zone from a matrix representing the zone
    explicit Zone(Eigen::Matrix <Bounds, Eigen::Dynamic, Eigen::Dynamic> &&value) : value(std::move(value)) {}

    //! @brief Construct a zone from a matrix representing the zone and the bound
    Zone(Eigen::Matrix <Bounds, Eigen::Dynamic, Eigen::Dynamic> value, Bounds m) :
            value(std::move(value)), M(std::move(m)) {
      maxConstraints.resize(this->value.cols() - 1);
      std::fill(maxConstraints.begin(), maxConstraints.end(), M.first);
    }

    /*!
     * @brief Construct a zone containing only the given valuation
     */
    explicit Zone(const std::vector<double> &valuation, Bounds M) : M(std::move(M)) {
      value.resize(valuation.size() + 1, valuation.size() + 1);
      value.fill(Bounds(std::numeric_limits<double>::max(), false));
      for (std::size_t i = 0; i < valuation.size(); ++i) {
        this->tighten(i, -1, Bounds{valuation.at(i), true});
        this->tighten(-1, i, Bounds{-valuation.at(i), true});
      }
      maxConstraints.resize(this->value.cols() - 1);
      std::fill(maxConstraints.begin(), maxConstraints.end(), this->M.first);
    }

    /*!
      Returns the number of the variables represented by this zone
      @returns the number of the variables
    */
    [[nodiscard]] inline std::size_t getNumOfVar() const noexcept {
      return value.cols() - 1;
    }

    //! @brief Make the zone of size `size` such that all the values are zero
    static Zone zero(int size) {
      static Zone zeroZone;
      if (zeroZone.value.cols() == size) {
        return zeroZone;
      }
      zeroZone.value.resize(size, size);
      zeroZone.value.fill(Bounds(0, true));
      return zeroZone;
    }

    /*!
     * @brief Make the zone of size `size` with no constraints
     */
    static Zone top(std::size_t size) {
      static Zone topZone;
      if (static_cast<std::size_t>(topZone.value.cols()) == size) {
        return topZone;
      }
      topZone.value.resize(size, size);
      topZone.value.fill(Bounds(std::numeric_limits<double>::max(), false));
      return topZone;
    }

    //! @brief add the constraint \f$x - y \le (c,s)\f$
    void tighten(ClockVariables x, ClockVariables y, Bounds c) {
      x++;
      y++;
      value(x, y) = std::min(value(x, y), c);
      close1(x);
      close1(y);
    }

    //! @brief Add a guard of a timed automaton
    void tighten(const Constraint &constraint) {
      switch (constraint.odr) {
        case learnta::Constraint::Order::ge:
          // lower bound
          this->tighten(-1, constraint.x, Bounds{-constraint.c, true});
          break;
        case learnta::Constraint::Order::gt:
          // lower bound
          this->tighten(-1, constraint.x, Bounds{-constraint.c, false});
          break;
        case learnta::Constraint::Order::le:
          // upper bound
          this->tighten(constraint.x, -1, Bounds{constraint.c, true});
          break;
        case learnta::Constraint::Order::lt:
          // upper bound
          this->tighten(constraint.x, -1, Bounds{constraint.c, false});
          break;
      }
    }

    //! @brief Add a set of guards of a timed automaton
    void tighten(const std::vector<Constraint> &constraints) {
      for (const auto &constraint: constraints) {
        this->tighten(constraint);
      }
    }

    void applyResets(const std::vector<std::pair<ClockVariables, std::variant<double, ClockVariables>>> &resets) {
      for (const auto &[resetVariable, updatedVariable]: resets) {
        this->unconstrain(resetVariable);
        if (updatedVariable.index() == 1) {
          if (resetVariable != std::get<ClockVariables>(updatedVariable)) {
            this->value(resetVariable + 1, std::get<ClockVariables>(updatedVariable) + 1) = Bounds{0.0, true};
            this->value(std::get<ClockVariables>(updatedVariable) + 1, resetVariable + 1) = Bounds{0.0, true};
          }
        } else {
          this->value(0, resetVariable + 1) = Bounds(-std::get<double>(updatedVariable), true);
          this->value(resetVariable + 1, 0) = Bounds(std::get<double>(updatedVariable), true);
        }
        canonize();
      }
    }

    /*!
     * @brief Make it the weakest precondition of the reset
     *
     * In addition to unconstrain the updated variables, we use the renaming information
     */
    void revertResets(const std::vector<std::pair<ClockVariables, std::variant<double, ClockVariables>>> &resets) {
      std::vector<ClockVariables> resetVariables;
      resetVariables.reserve(resets.size());
      std::unordered_map<ClockVariables, ClockVariables> reverseAssignments;
      for (const auto &[resetVariable, updatedVariable]: resets) {
        resetVariables.push_back(resetVariable);
        if (updatedVariable.index() == 1 && resetVariable != std::get<ClockVariables>(updatedVariable)) {
          reverseAssignments[std::get<ClockVariables>(updatedVariable)] = resetVariable;
        }
      }
      for (const auto &resetVariable: boost::adaptors::reverse(resetVariables)) {
        this->unconstrain(resetVariable);
        auto it = reverseAssignments.find(resetVariable);
        if (it != reverseAssignments.end()) {
          this->value(it->first + 1, it->second + 1) = Bounds{0.0, true};
          this->value(it->second + 1, it->first + 1) = Bounds{0.0, true};
        }
        canonize();
      }
    }

    /*!
     * @brief Returns the intersection of two zones
     */
    Zone operator&&(const Zone &another) const {
      assert(this->value.cols() == another.value.cols());
      assert(this->value.rows() == another.value.rows());
      auto result = Zone{this->value.array().min(another.value.array())};
      result.M = this->M;
      result.canonize();

      return result;
    }

    /*!
     * @brief Assign the intersection of two zones
     */
    Zone operator&=(const Zone &another) {
      assert(this->value.cols() == another.value.cols());
      assert(this->value.rows() == another.value.rows());
      this->value = this->value.cwiseMin(another.value);
      canonize();

      return *this;
    }

    /*!
     * @brief Returns the juxtaposition of two zones
     */
    Zone operator^(const Zone &another) const {
      Zone result;
      const size_t N = this->getNumOfVar();
      const size_t O = another.getNumOfVar();
      result = Zone::top(N + O);
      // Copy \f$\mathbb{T}'\f$
      result.value.block(N, N, O, O) = another.value.block(1, 1, O, O);
      result.value.block(0, N + 1, 1, O - 1) = another.value.block(0, 1, 1, O - 1);
      result.value.block(N + 1, 0, O - 1, 1) = another.value.block(1, 0, O - 1, 1);
      // Copy \f$\mathbb{T}\f$
      result.value.block(0, 0, N + 1, N + 1) = this->value;
      result.canonize();

      return result;
    }

    /*!
     * @brief Return a clock valuation in this zone
     */
    [[nodiscard]] std::vector<double> sample() {
      this->canonize();
      assert(this->isSatisfiableNoCanonize());
      std::vector<double> valuation;
      std::size_t N = this->getNumOfVar();
      valuation.resize(N);
      for (std::size_t i = 0; i < N; i++) {
        Bounds lowerBound = this->value(0, i + 1);
        Bounds upperBound = this->value(i + 1, 0);
        if (isPoint(upperBound, lowerBound)) {
          valuation[i] = upperBound.first;
        } else {
          double lower = std::max(0.0, -lowerBound.first);
          double upper = upperBound.first;
          for (std::size_t j = 0; j < i; j++) {
            Bounds tmpLowerBound = this->value(j + 1, i + 1);
            Bounds tmpUpperBound = this->value(i + 1, j + 1);
            lower = std::max(lower, -tmpLowerBound.first + valuation[j]);
            upper = std::min(upper, tmpUpperBound.first + valuation[j]);
          }
          assert(lower <= upper);
          if (lowerBound.second) {
            valuation[i] = lower;
          } else if (upper - lower > 0.5) {
            valuation[i] = lower + 0.5;
          } else {
            valuation[i] = (lower + upper) * 0.5;
          }
        }
      }

      return valuation;
    }

    //! @brief Close using only x
    void close1(ClockVariables x) {
      for (int i = 0; i < value.rows(); i++) {
        value.row(i) = value.row(i).array().min(value.row(x).array() + value(i, x));
        //      for (int j = 0; j < value.cols(); j++) {
        //        value(i, j) = std::min(value(i, j), value(i, x) + value(x, j));
        //      }
      }
    }

    //! @brief Assign a constant value to the clock variable x
    void reset(ClockVariables x) {
      // 0 is the special variable here
      x++;
      value(0, x) = Bounds(-0.0, true);
      value(x, 0) = Bounds(0.0, true);
      value.col(x).tail(value.rows() - 1) = value.col(0).tail(value.rows() - 1);
      value.row(x).tail(value.cols() - 1) = value.row(0).tail(value.cols() - 1);
    }

    /*!
     * @brief Unconstrain the constraint on this clock
     */
    void unconstrain(ClockVariables x) {
      // 0 is the special variable here
      x++;
      value.col(x).fill(Bounds(std::numeric_limits<double>::max(), false));
      value.row(x).fill(Bounds(std::numeric_limits<double>::max(), false));
    }

    /*!
     * @brief Assign the strongest post-condition of the delay
     *
     * @note We allow time elapse of duration zero
     */
    void elapse() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::max(), false);
      value.col(0).fill(Bounds(infinity));
    }

    /*!
     * @brief Assign the weakest pre-condition of the delay
     *
     * @note We allow time elapse of duration zero
     *
     * @note Since clock variables cannot be negative, SAT(elapsed(Z) && Z') does not imply SAT(Z && reverseElapsed(Z'))
     */
    void reverseElapse() {
      value.row(0).fill(Bounds(0, true));
    }

    /*!
     * @brief make the zone canonical
    */
    void canonize() {
      for (int k = 0; k < value.cols(); k++) {
        close1(k);
      }
    }

    /*!
     * @brief check if the zone is satisfiable
     */
    bool isSatisfiable() {
      canonize();
      return this->isSatisfiableNoCanonize();
    }

    /*!
     * @brief check if the zone is satisfiable
     *
     * @pre The zone is canonical
     */
    [[nodiscard]] bool isSatisfiableNoCanonize() const {
      return (value + value.transpose()).minCoeff() >= Bounds(0.0, true);
    }

    /*!
     * @brief check if the zone is satisfiable
     */
    explicit operator bool() {
      return isSatisfiable();
    }

    /*!
      @brief truncate the constraints compared with a constant greater than or equal to M
    */
    void abstractize() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::max(), false);
      for (auto it = value.data(); it < value.data() + value.size(); it++) {
        if (*it > Bounds{M.first, true}) {
          *it = Bounds(infinity);
        } else if (*it < Bounds{-M.first, false}) {
          *it = Bounds(-M.first, false);
        }
      }
    }

    /*!
     * @brief Extrapolate the zone using the diagonal extrapolation based on maximum constants
     *
     * See [Behrmann+, TACAS'04] for the detail.
     * @note We are currently using this simple extrapolation for simplicity. If we need more speed, we will consider using more sophisticated methods, e.g., LU extrapolation.
    */
    void extrapolate() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::max(), false);
      for (std::size_t i = 0; i < this->maxConstraints.size(); ++i) {
        if (value(i + 1, 0).first > this->maxConstraints.at(i)) {
          value(i + 1, 0) = infinity;
        }
        if (-value(0, i + 1).first > this->maxConstraints.at(i)) {
          value(0, i + 1) = Bounds{-this->maxConstraints.at(i), false};
        }
        for (std::size_t j = 0; j < this->maxConstraints.size(); ++j) {
          if (value(i + 1, j + 1).first > this->maxConstraints.at(i)) {
            value(i + 1, j + 1) = infinity;
          } else if (-value(0, i + 1).first > this->maxConstraints.at(i)) {
            value(i + 1, j + 1) = infinity;
          } else if (-value(0, j + 1).first > this->maxConstraints.at(j)) {
            value(i + 1, j + 1) = infinity;
          }
        }
      }
    }

    /*!
      @brief make the zone unsatisfiable
    */
    void makeUnsat() {
      value(0, 0) = Bounds(-std::numeric_limits<double>::infinity(), false);
    }

    /*!
     * @brief Return if this zone includes the given zone
     *
     * @pre both this and the given zones are canonized
     */
    [[nodiscard]] bool includes(const Zone &zone) const {
      return this->value.cwiseMax(zone.value) == this->value;
    };

    /*!
     * @brief Check the equivalence of two zones
     *
     * @note We assume that the diagonal elements are equal.
     */
    bool operator==(const Zone &z) const {
      return value == z.value;
    }

    /*!
      * @brief Check the equivalence of two zones
      *
      * @note We do not assume that the diagonal elements are equal.
      */
    [[nodiscard]] bool equalIgnoreZero(Zone z) const {
      z.value(0, 0) = value(0, 0);
      return value == z.value;
    }

    /*!
     * @brief Check the equivalence of two zones
     *
     * @note We do not assume that the diagonal elements are equal.
     */
    [[nodiscard]] bool strictEqual(Zone z) const {
      z.value.diagonal() = value.diagonal();
      return value == z.value;
    }
  };

  //! @brief Print the zone
  static inline std::ostream &print(std::ostream &os, const learnta::Zone &zone) {
    for (int i = 0; i < zone.value.cols(); ++i) {
      for (int j = 0; j < zone.value.rows(); ++j) {
        print(os, zone.value(i, j));
        os << " ";
      }
      os << "\n";
    }

    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const learnta::Zone &zone) {
    return learnta::print(os, zone);
  }

  inline std::size_t hash_value(learnta::Zone const &zone) {
    std::size_t seed = zone.value.array().size();
    const auto asVector = zone.value.array();

    union DI {
      double asD;
      uint64_t asI;
    };

    for (auto it = asVector.data(); it != asVector.data() + asVector.size(); it++) {
      DI value{};
      value.asD = it->first;
      seed ^= it->second + value.asI + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
}
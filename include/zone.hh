#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <boost/unordered_map.hpp>

#include "common_types.hh"
#include "bounds.hh"
#include "constraint.hh"
// #include "constraint.hh"

#include <Eigen/Core>
#include <utility>

namespace learnta {
  static inline bool isPoint(const Bounds &upperBound, const Bounds &lowerBound) {
    auto[upperConstant, upperEq] = upperBound; // i - j \le (c, s)
    auto[lowerConstant, lowerEq] = lowerBound; // j - i \le (c, s)
    lowerConstant = -lowerConstant;
    return lowerConstant == upperConstant and upperEq and lowerEq;
  }

  static inline bool isUnitOpen(const Bounds &upperBound, const Bounds &lowerBound) {
    auto[upperConstant, upperEq] = upperBound; // i - j \le (c, s)
    auto[lowerConstant, lowerEq] = lowerBound; // j - i \le (c, s)
    lowerConstant = -lowerConstant;
    return lowerConstant + 1 == upperConstant and (not upperEq) and (not lowerEq);
  }

  /*!
    @brief Implementation of a zone with DBM

    @note internally, the variable 0 is used for the constant while externally, the actual clock variable is 0 origin, i.e., the variable 0 for the user is the variable 1 internally. So, we need increment or decrement to fill the gap.
  */
  struct Zone {
    using Tuple = std::tuple<std::vector<Bounds>, Bounds>;
    Eigen::Matrix<Bounds, Eigen::Dynamic, Eigen::Dynamic> value;
    Bounds M;

    Zone() = default;

    explicit Zone(const Eigen::Matrix<Bounds, Eigen::Dynamic, Eigen::Dynamic> &value) :
            Zone(Eigen::Matrix<Bounds, Eigen::Dynamic, Eigen::Dynamic>(value)) {}

    explicit Zone(Eigen::Matrix<Bounds, Eigen::Dynamic, Eigen::Dynamic> &&value) : value(std::move(value)) {}

    Zone(Eigen::Matrix<Bounds, Eigen::Dynamic, Eigen::Dynamic> value, Bounds m) :
            value(std::move(value)), M(std::move(m)) {}

    /*!
     * @brief Construct a zone containing only the given valuation
     */
    explicit Zone(const std::vector<double> &valuation, Bounds M) : M(std::move(M)) {
      value.resize(valuation.size() + 1, valuation.size() + 1);
      value.fill(Bounds(std::numeric_limits<double>::max(), false));
      for (int i = 0; i < valuation.size(); ++i) {
        this->tighten(i, -1, Bounds{valuation.at(i), true});
        this->tighten(-1, i, Bounds{-valuation.at(i), true});
      }
    }

    /*!
      Returns the number of the variables represented by this zone
      @returns the number of the variables
    */
    [[nodiscard]] inline std::size_t getNumOfVar() const noexcept {
      return value.cols() - 1;
    }

    inline void cutVars(std::shared_ptr<Zone> &out, std::size_t from, std::size_t to) {
      out = std::make_shared<Zone>();
      out->value.resize(to - from + 2, to - from + 2);
      out->value.block(0, 0, 1, 1) << Bounds(0, true);
      out->value.block(1, 1, to - from + 1, to - from + 1) = value.block(from + 1, from + 1, to - from + 1,
                                                                         to - from + 1);
      out->value.block(1, 0, to - from + 1, 1) = value.block(from + 1, 0, to - from + 1, 1);
      out->value.block(0, 1, 1, to - from + 1) = value.block(0, from + 1, 1, to - from + 1);
      out->M = M;
    }

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
      if (topZone.value.cols() == size) {
        return topZone;
      }
      topZone.value.resize(size, size);
      topZone.value.fill(Bounds(std::numeric_limits<double>::max(), false));
      return topZone;
    }

    [[nodiscard]] std::tuple<std::vector<Bounds>, Bounds> toTuple() const {
      // omit (0,0)
      return {std::vector<Bounds>(value.data() + 1, value.data() + value.size()), M};
    }

    //! @brief add the constraint x - y \le (c,s)
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

    /*!
     * @brief Returns the intersection of two zones
     */
    Zone operator&&(const Zone &another) {
      assert(this->value.cols() == another.value.cols());
      assert(this->value.rows() == another.value.rows());
      return Zone{this->value.array().min(another.value.array())};
    }

    /*!
     * @brief Assign the intersection of two zones
     */
    Zone operator&=(const Zone &another) {
      assert(this->value.cols() == another.value.cols());
      assert(this->value.rows() == another.value.rows());
      this->value = this->value.cwiseMin(another.value);
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
      for (int i = 0; i < N; i++) {
        Bounds lowerBound = this->value(0, i + 1);
        Bounds upperBound = this->value(i + 1, 0);
        if (isPoint(upperBound, lowerBound)) {
          valuation[i] = upperBound.first;
        } else {
          double lower = -lowerBound.first;
          double upper = upperBound.first;
          for (int j = 0; j < i; j++) {
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

    void close1(ClockVariables x) {
      for (int i = 0; i < value.rows(); i++) {
        value.row(i) = value.row(i).array().min(value.row(x).array() + value(i, x));
        //      for (int j = 0; j < value.cols(); j++) {
        //        value(i, j) = std::min(value(i, j), value(i, x) + value(x, j));
        //      }
      }
    }

    // The reset value is always (0, \le)
    void reset(ClockVariables x) {
      // 0 is the special variable here
      x++;
      value(0, x) = Bounds(0, true);
      value(x, 0) = Bounds(0, true);
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

    void elapse() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
      value.col(0).fill(Bounds(infinity));
      for (int i = 0; i < value.row(0).size(); ++i) {
        value.row(0)[i].second = false;
      }
    }

    void reverseElapse() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
      value.row(0).fill(Bounds(infinity));
      for (int i = 0; i < value.col(0).size(); ++i) {
        value.col(0)[i].second = false;
      }
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
     */
    [[nodiscard]] bool isSatisfiableNoCanonize() const {
      return (value + value.transpose()).minCoeff() >= Bounds(0.0, true);
    }

    explicit operator bool() {
      return isSatisfiable();
    }

    /*!
      @brief truncate the constraints compared with a constant greater than or equal to M
    */
    void abstractize() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
      for (auto it = value.data(); it < value.data() + value.size(); it++) {
        if (*it > Bounds{M.first, true}) {
          *it = Bounds(infinity);
        } else if (*it < Bounds{-M.first, false}) {
          *it = Bounds(-M.first, false);
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

    /*
    //! @brief make the strongest guard including the zone
    std::vector<Constraint> makeGuard() {
      std::vector<Constraint> guard;
      canonize();
      abstractize();
      for (int i = 0; i < getNumOfVar(); ++i) {
        // the second element of Bound is true if the constraint is not strict, i.e., LE or GE.
        Bounds lowerBound = value(0, i + 1);
        if (lowerBound.first > 0 or (lowerBound.first == 0 and lowerBound.second == false)) {
          if (lowerBound.second) {
            guard.push_back(ConstraintMaker(i) >= lowerBound.first);
          } else {
            guard.push_back(ConstraintMaker(i) > lowerBound.first);
          }
        }
        Bounds upperBound = value(i + 1, 0);
        if (upperBound < M) {
          if (upperBound.second) {
            guard.push_back(ConstraintMaker(i) <= upperBound.first);
          } else {
            guard.push_back(ConstraintMaker(i) < upperBound.first);
          }
        }
      }
      return guard;
    }*/

    bool operator==(Zone z) const {
      z.value(0, 0) = value(0, 0);
      return value == z.value;
    }
  };

  static inline std::ostream &print(std::ostream &os, const learnta::Zone &zone) {
    for (int i = 0; i < zone.value.cols(); ++i) {
      for (int j = 0; j < zone.value.rows(); ++j) {
        print(os, zone.value(i, j));
        os << " ";
      }
    }

    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const learnta::Zone &zone) {
    return learnta::print(os, zone);
  }
}

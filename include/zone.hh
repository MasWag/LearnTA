#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <boost/unordered_map.hpp>

#include "common_types.hh"
#include "bounds.hh"
// #include "constraint.hh"

#include <Eigen/Core>

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
      Returns the number of the variables represented by this zone
      @returns the number of the variables
    */
    inline std::size_t getNumOfVar() const noexcept {
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
    static Zone top(int size) {
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

    /*!
     * @brief Returns the intersection of two zones
     */
    Zone operator&&(const Zone &another) {
      assert(this->value.cols() == another.value.cols());
      assert(this->value.rows() == another.value.rows());
      return Zone{this->value.array().min(another.value.array())};
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
      // 0 is the special varibale here
      x++;
      value(0, x) = Bounds(0, true);
      value(x, 0) = Bounds(0, true);
      value.col(x).tail(value.rows() - 1) = value.col(0).tail(value.rows() - 1);
      value.row(x).tail(value.cols() - 1) = value.row(0).tail(value.cols() - 1);
    }

    void elapse() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
      value.col(0).fill(Bounds(infinity));
      for (int i = 0; i < value.row(0).size(); ++i) {
        value.row(0)[i].second = false;
      }
    }

    /*
      @brief make the zone canonical
    */
    void canonize() {
      for (int k = 0; k < value.cols(); k++) {
        close1(k);
      }
    }

    /*
      @brief check if the zone is satisfiable
    */
    bool isSatisfiable() {
      canonize();
      return (value + value.transpose()).minCoeff() >= Bounds(0.0, true);
    }

    explicit operator bool() {
      return isSatisfiable();
    }

    /*
      @brief truncate the constraints compared with a constant greater than or equal to M
    */
    void abstractize() {
      static constexpr Bounds infinity = Bounds(std::numeric_limits<double>::infinity(), false);
      for (auto it = value.data(); it < value.data() + value.size(); it++) {
        if (*it >= M) {
          *it = Bounds(infinity);
        }
      }
    }

    /*!
      @brief make the zone unsatisfiable
    */
    void makeUnsat() {
      value(0, 0) = Bounds(-std::numeric_limits<double>::infinity(), false);
    }

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
}

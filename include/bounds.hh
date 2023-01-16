/**
 * @author Masaki Waga
 * @date 2022/03/06.
 */

#pragma once

#include <iostream>
#include <utility>

namespace learnta {
  using Bounds = std::pair<double, bool>;

  /*!
   * @brief Check if the upper and lower bounds define a point.
   */
  static inline bool isPoint(const Bounds &upperBound, const Bounds &lowerBound) {
    auto [upperConstant, upperEq] = upperBound; // i - j \le (c, s)
    auto [lowerConstant, lowerEq] = lowerBound; // j - i \le (c, s)
    lowerConstant = -lowerConstant;
    return lowerConstant == upperConstant and upperEq and lowerEq;
  }

  static inline bool isUnitOpen(const Bounds &upperBound, const Bounds &lowerBound) {
    auto [upperConstant, upperEq] = upperBound; // i - j \le (c, s)
    auto [lowerConstant, lowerEq] = lowerBound; // j - i \le (c, s)
    lowerConstant = -lowerConstant;
    return lowerConstant + 1 == upperConstant and (not upperEq) and (not lowerEq);
  }

  static inline bool isSimple(const Bounds &upperBound, const Bounds &lowerBound) {
    return isPoint(upperBound, lowerBound) || isUnitOpen(upperBound, lowerBound);
  }
}

static inline learnta::Bounds operator+(const learnta::Bounds &a, const learnta::Bounds &b) {
  return {a.first + b.first, a.second && b.second};
}

static inline learnta::Bounds operator-(const learnta::Bounds &a, const learnta::Bounds &b) {
  return {a.first - b.first, a.second && b.second};
}

static inline learnta::Bounds operator-(const learnta::Bounds &a) {
  return {-a.first, a.second};
}

static inline void operator+=(learnta::Bounds &a, const learnta::Bounds b) {
  a.first += b.first;
  a.second = a.second && b.second;
}

namespace learnta {
  static inline std::ostream &print(std::ostream &os, const learnta::Bounds &b) {
    os << "(" << b.first << ", " << b.second << ")";
    return os;
  }

  static inline std::ostream &print(std::ostream &os, learnta::Bounds &&b) {
    os << "(" << b.first << ", " << b.second << ")";
    return os;
  }
}

static inline std::ostream &operator<<(std::ostream &os, const learnta::Bounds &b) {
  return learnta::print(os, b);
}

namespace boost {
  // This overload is a hack for Boost test
  static inline std::ostream &operator<<(std::ostream &os, const learnta::Bounds &b) {
    return learnta::print(os, b);
  }

  static inline std::ostream &operator<<(std::ostream &os, learnta::Bounds &&b) {
    return learnta::print(os, b);
  }
}

namespace boost::test_tools::tt_detail::impl {
  // This overload is a hack for Boost test
  static inline std::ostream &operator<<(std::ostream &os, const learnta::Bounds &b) {
    return learnta::print(os, b);
  }

  static inline std::ostream &operator<<(std::ostream &os, learnta::Bounds &&b) {
    return learnta::print(os, b);
  }
}

namespace boost::detail::has_left_shift_impl {
  // This overload is a hack for Boost test
  static inline std::ostream &operator<<(std::ostream &os, const learnta::Bounds &b) {
    return learnta::print(os, b);
  }

  static inline std::ostream &operator<<(std::ostream &os, learnta::Bounds &&b) {
    return learnta::print(os, b);
  }
}

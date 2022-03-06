/**
 * @author Masaki Waga
 * @date 2022/03/06.
 */

#pragma once

#include <utility>

namespace learnta {
  using Bounds = std::pair<double, bool>;
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

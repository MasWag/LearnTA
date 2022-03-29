#pragma once

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <vector>

#include "common_types.hh"
#include "bounds.hh"

namespace learnta {

//! @brief The return values of comparison of two values. Similar to strcmp.
  enum class Order {
    LT, EQ, GT
  };

  inline bool toBool(Order odr) { return odr == Order::EQ; }

//! @brief A constraint in a guard of transitions
  struct Constraint {
    enum class Order {
      lt, le, ge, gt
    };

    ClockVariables x;
    Order odr;
    int c;

    [[nodiscard]] bool satisfy(double d) const {
      switch (odr) {
        case Order::lt:
          return d < c;
        case Order::le:
          return d <= c;
        case Order::gt:
          return d > c;
        case Order::ge:
          return d >= c;
      }
      return false;
    }

    using Interpretation = std::vector<double>;

    learnta::Order operator()(Interpretation val) const {
      if (satisfy(val.at(x))) {
        return learnta::Order::EQ;
      } else if (odr == Order::lt || odr == Order::le) {
        return learnta::Order::GT;
      } else {
        return learnta::Order::LT;
      }
    }

    bool operator==(Constraint another) const {
      return this->odr == another.odr && this->c == another.c && this->x == another.x;
    }

    [[nodiscard]] bool isWeaker(Constraint another) const {
      if (this->x != another.x) {
        return false;
      }
      const bool thisLess = this->odr == Order::le || this->odr == Order::lt;
      const bool anotherLess = another.odr == Order::le || another.odr == Order::lt;
      Bounds thisBounds, anotherBounds;
      switch (this->odr) {
        case Order::le:
          thisBounds = Bounds{c, true};
          break;
        case Order::lt:
          thisBounds = Bounds{c, false};
          break;
        case Order::ge:
          thisBounds = Bounds{-c, true};
          break;
        case Order::gt:
          thisBounds = Bounds{-c, false};
          break;
      }
      switch (another.odr) {
        case Order::le:
          anotherBounds = Bounds{another.c, true};
          break;
        case Order::lt:
          anotherBounds = Bounds{another.c, false};
          break;
        case Order::ge:
          anotherBounds = Bounds{-another.c, true};
          break;
        case Order::gt:
          anotherBounds = Bounds{-another.c, false};
          break;
      }

      return thisLess == anotherLess && anotherBounds <= thisBounds;
    }
  };

  static inline bool isWeaker(const std::vector<Constraint> &left, const std::vector<Constraint> &right) {
    return std::all_of(left.begin(), left.end(), [&] (const auto &leftGuard){
      return std::any_of(right.begin(), right.end(), [&] (const auto &rightGuard) {
        return leftGuard.isWeaker(rightGuard);
      });
    });
  }

  static inline std::ostream &operator<<(std::ostream &os,
                                         const Constraint::Order &odr) {
    switch (odr) {
      case Constraint::Order::lt:
        os << "<";
        break;
      case Constraint::Order::le:
        os << "<=";
        break;
      case Constraint::Order::ge:
        os << ">=";
        break;
      case Constraint::Order::gt:
        os << ">";
        break;
    }
    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const Constraint &p) {
    os << "x" << int(p.x) << " " << p.odr << " " << p.c;
    return os;
  }

// An interface to write an inequality constraint easily
  class ConstraintMaker {
    ClockVariables x;

  public:
    explicit ConstraintMaker(ClockVariables x) : x(x) {}

    Constraint operator<(int c) {
      return Constraint{x, Constraint::Order::lt, c};
    }

    Constraint operator<=(int c) {
      return Constraint{x, Constraint::Order::le, c};
    }

    Constraint operator>(int c) {
      return Constraint{x, Constraint::Order::gt, c};
    }

    Constraint operator>=(int c) {
      return Constraint{x, Constraint::Order::ge, c};
    }
  };

/*!
  @brief remove any inequality x > c or x >= c
 */
  static inline void widen(std::vector<Constraint> &guard) {
    guard.erase(std::remove_if(guard.begin(), guard.end(),
                               [](Constraint g) {
                                 return g.odr == Constraint::Order::ge ||
                                        g.odr == Constraint::Order::gt;
                               }),
                guard.end());
  }
}
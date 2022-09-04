#pragma once

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <vector>

#include <boost/unordered_map.hpp>

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

    [[nodiscard]] Constraint negate() const {
      switch (odr) {
        case Order::lt:
          return Constraint{x, learnta::Constraint::Order::ge, c};
        case Order::le:
          return Constraint{x, learnta::Constraint::Order::gt, c};
        case Order::gt:
          return Constraint{x, learnta::Constraint::Order::le, c};
        case Order::ge:
          return Constraint{x, learnta::Constraint::Order::lt, c};
      }
    }
  };

  inline int orderToInt(learnta::Constraint::Order order) {
    switch (order) {
      case learnta::Constraint::Order::lt:
        return 0;
      case learnta::Constraint::Order::le:
        return 1;
      case learnta::Constraint::Order::ge:
        return 2;
      case learnta::Constraint::Order::gt:
        return 3;
      default:
        return 4;
    }
  }

  inline std::size_t hash_value(const Constraint guard) {
    return boost::hash_value(std::make_tuple(guard.c, orderToInt(guard.odr), guard.x));
  }

  static inline bool isWeaker(const std::vector<Constraint> &left, const std::vector<Constraint> &right) {
    return std::all_of(left.begin(), left.end(), [&](const auto &leftGuard) {
      return std::any_of(right.begin(), right.end(), [&](const auto &rightGuard) {
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

  inline std::vector<Constraint> negate(const std::vector<Constraint> &constraints) {
    std::vector<Constraint> result;
    result.reserve(constraints.size());
    std::transform(constraints.begin(), constraints.end(), std::back_inserter(result), [&](const auto &constraint) {
      return constraint.negate();
    });

    return result;
  }

  inline bool satisfiable(const std::vector<Constraint> &constraints) {
    std::vector<Bounds> upperBounds;
    std::vector<Bounds> lowerBounds;

    const auto resizeBounds = [&] (const ClockVariables x) {
      if (x >= upperBounds.size()) {
        const auto oldSize = upperBounds.size();
        upperBounds.resize(x + 1);
        lowerBounds.resize(x + 1);
        for (auto i = oldSize; i <= x; ++i) {
          upperBounds.at(i) = Bounds(std::numeric_limits<double>::max(), false);
          lowerBounds.at(i) = Bounds(0, true);
        }
      }
    };

    const auto toBound = [&] (const Constraint &constraint) {
      switch (constraint.odr) {
        case learnta::Constraint::Order::le:
        case learnta::Constraint::Order::ge:
          return Bounds{constraint.c, true};
        case learnta::Constraint::Order::lt:
        case learnta::Constraint::Order::gt:
          return Bounds{constraint.c, false};
      }
    };

    for (const auto &constraint: constraints) {
      resizeBounds(constraint.x);
      Bounds bound = toBound(constraint);

      switch (constraint.odr) {
        case learnta::Constraint::Order::le:
        case learnta::Constraint::Order::lt:
          if (lowerBounds.at(constraint.x) < bound) {
            lowerBounds.at(constraint.x) = bound;
          }
          break;
        case learnta::Constraint::Order::ge:
        case learnta::Constraint::Order::gt:
          if (upperBounds.at(constraint.x) > bound) {
            upperBounds.at(constraint.x) = bound;
          }
          break;
      }
    }

      for (int i = 0; i < lowerBounds.size(); ++i) {
        if (lowerBounds.at(i).first > upperBounds.at(i).first ||
            (lowerBounds.at(i) == upperBounds.at(i) && !lowerBounds.at(i).second)) {
          return false;
        }
      }

      return true;
    }


  inline std::vector<std::vector<Constraint>> negate(const std::vector<std::vector<Constraint>> &dnfConstraints) {
    std::vector<std::vector<Constraint>> cnfNegated;
    cnfNegated.reserve(dnfConstraints.size());
    std::transform(dnfConstraints.begin(), dnfConstraints.end(), std::back_inserter(cnfNegated), [&](const auto &constraints) {
      return negate(constraints);
    });
    std::vector<std::vector<Constraint>> dnfNegated;
    for (const auto &disjunct: cnfNegated) {
      if (dnfNegated.empty()) {
        dnfNegated.reserve(disjunct.size());
        std::transform(disjunct.begin(), disjunct.end(), std::back_inserter(dnfNegated), [](const auto &constraint) {
          return std::vector<Constraint> {constraint};
        });
      } else {
        std::vector<std::vector<Constraint>> newDnfNegated;
        for (const auto &constraint: disjunct) {
          for (auto conjunct: dnfNegated) {
            conjunct.push_back(constraint);
            if (satisfiable(conjunct)) {
              newDnfNegated.push_back(std::move(conjunct));
            }
          }
        }
        dnfNegated = std::move(newDnfNegated);
      }
    }

    return dnfNegated;
  }
}
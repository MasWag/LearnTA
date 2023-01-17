#pragma once

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <vector>
#include <unordered_map>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

#include "common_types.hh"
#include "bounds.hh"

namespace learnta {

//! @brief The return values of comparison of two values. Similar to strcmp.
  enum class Order {
    LT, EQ, GT
  };

  inline bool toBool(Order odr) { return odr == Order::EQ; }

  using IntBounds = std::pair<int, bool>;

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

    [[nodiscard]] bool satisfy(std::vector<double> valuation) const {
      return satisfy(valuation.at(x));
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

    [[nodiscard]] inline bool isUpperBound() const {
      return this->odr == Order::le || this->odr == Order::lt;
    }

    /*!
     * @brief Check if this constraint is weaker than the given one.
     *
     * @returns false if the compared variables are different
     * @returns false if the compared constraints bound from the different order (i.e., upper vs. lower)
     */
    [[nodiscard]] bool isWeaker(const Constraint &another) const {
      if (this->x != another.x) {
        return false;
      }
      const bool thisLess = this->isUpperBound();
      const bool anotherLess = another.isUpperBound();
      const Bounds thisBounds = this->toDBMBound();
      const Bounds anotherBounds = another.toDBMBound();

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
        default:
          throw std::range_error("Invalid order is used");
      }
    }

    [[nodiscard]] inline Bounds lowerBoundDurationToSatisfy(std::vector<double> valuation) const {
      switch (odr) {
        case Order::lt:
        case Order::le:
          // Bounded from above
          return valuation.at(this->x) <= this->c ? Bounds{0.0, true} : Bounds{-std::numeric_limits<double>::infinity(), false};
        case Order::ge:
          // Bounded from below
          return std::min(Bounds{0.0, true}, Bounds{-this->c + valuation.at(this->x), true});
        case Order::gt:
          // Bounded from below
          return std::min(Bounds{0.0, true}, Bounds{-this->c + valuation.at(this->x), false});
      }
      return Bounds{-std::numeric_limits<double>::infinity(), false};
    }

    [[nodiscard]] IntBounds toBound() const {
      switch (odr) {
        case learnta::Constraint::Order::le:
        case learnta::Constraint::Order::ge:
          return IntBounds{c, true};
        case learnta::Constraint::Order::lt:
        case learnta::Constraint::Order::gt:
          return IntBounds{c, false};
        default:
          throw std::range_error("Invalid order is used");
      }
    };


    [[nodiscard]] Bounds toDBMBound() const {
      switch (odr) {
        case learnta::Constraint::Order::le:
          return Bounds{c, true};
        case learnta::Constraint::Order::ge:
          return Bounds{-c, true};
        case learnta::Constraint::Order::lt:
          return Bounds{c, false};
        case learnta::Constraint::Order::gt:
          return Bounds{-c, false};
        default:
          throw std::range_error("Invalid order is used");
      }
    };
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

  static inline std::ostream &operator<<(std::ostream &os, const Constraint::Order &odr) {
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

  inline std::vector<Constraint> negateAll(const std::vector<Constraint> &constraints) {
    std::vector<Constraint> result;
    result.reserve(constraints.size());
    std::transform(constraints.begin(), constraints.end(), std::back_inserter(result), [&](const auto &constraint) {
      return constraint.negate();
    });

    return result;
  }

  inline std::vector<Constraint> conjunction(const std::vector<Constraint> &left, const std::vector<Constraint> &right) {
    std::vector<Constraint> result = left;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), right.begin(), right.end());

    return result;
  }

  inline auto toBounds(const std::vector<Constraint> &constraints) {
    std::vector<IntBounds> upperBounds;
    std::vector<IntBounds> lowerBounds;
    const auto resizeBounds = [&](const ClockVariables x) {
      if (x >= upperBounds.size()) {
        const auto oldSize = upperBounds.size();
        upperBounds.resize(x + 1);
        lowerBounds.resize(x + 1);
        for (auto i = oldSize; i <= x; ++i) {
          upperBounds.at(i) = IntBounds(std::numeric_limits<int>::max(), false);
          lowerBounds.at(i) = IntBounds(0, true);
        }
      }
    };

    for (const auto &constraint: constraints) {
      resizeBounds(constraint.x);
      const auto bound = constraint.toBound();

      if (constraint.isUpperBound()) {
        if (upperBounds.at(constraint.x) > bound) {
          upperBounds.at(constraint.x) = bound;
        }
      } else {
        if (-lowerBounds.at(constraint.x) > -bound) {
          lowerBounds.at(constraint.x) = bound;
        }
      }
    }

    return std::make_pair(upperBounds, lowerBounds);
  }

  /*!
   * @brief Return the variables that are bounded by a simple constraint, i.e., c < x < c + 1 or x = c
   *
   * @return list of variables with simple bounds in the ascending order
   */
  inline std::vector<ClockVariables> simpleVariables(const std::vector<Constraint> &constraints) {
    std::vector<ClockVariables> result;
    const auto &[upperBounds, lowerBounds] = toBounds(constraints);
    assert(upperBounds.size() == lowerBounds.size());
    result.reserve(upperBounds.size());
    for (std::size_t i = 0; i < upperBounds.size(); ++i) {
      if (isSimple(upperBounds.at(i), -lowerBounds.at(i))) {
        result.push_back(i);
      }
    }

    assert(is_ascending(result));
    return result;
  }

  inline bool satisfiable(const std::vector<Constraint> &constraints) {
    const auto &[upperBounds, lowerBounds] = toBounds(constraints);

    for (std::size_t i = 0; i < lowerBounds.size(); ++i) {
      if (lowerBounds.at(i).first > upperBounds.at(i).first ||
          (lowerBounds.at(i).first == upperBounds.at(i).first && !(lowerBounds.at(i).second && upperBounds.at(i).second))) {
        return false;
      }
    }

    return true;
  }

    inline std::vector<Constraint> simplify(const std::vector<Constraint> &constraints) {
    const auto &[upperBounds, lowerBounds] = toBounds(constraints);
    std::vector<Constraint> result;
    result.reserve(upperBounds.size() + lowerBounds.size());

    for (std::size_t i = 0; i < lowerBounds.size(); ++i) {
      if (lowerBounds.at(i) != IntBounds(0, true)) {
        if (lowerBounds.at(i).second) {
          result.push_back(ConstraintMaker(i) >= lowerBounds.at(i).first);
        } else {
          result.push_back(ConstraintMaker(i) > lowerBounds.at(i).first);
        }
      }
    }

    for (std::size_t i = 0; i < upperBounds.size(); ++i) {
      if (upperBounds.at(i) != IntBounds(std::numeric_limits<int>::max(), false)) {
        if (upperBounds.at(i).second) {
          result.push_back(ConstraintMaker(i) <= upperBounds.at(i).first);
        } else {
          result.push_back(ConstraintMaker(i) < upperBounds.at(i).first);
        }
      }
    }

    return result;
  }

  [[nodiscard]] inline Bounds lowerBoundDurationToSatisfy(const std::vector<Constraint> &guard, const std::vector<double> &valuation) {
    std::vector<Bounds> lowerBoundDurations;
    lowerBoundDurations.reserve(guard.size());
    std::transform(guard.begin(), guard.end(), std::back_inserter(lowerBoundDurations), [&] (const auto &constraint) {
      return constraint.lowerBoundDurationToSatisfy(valuation);
    });

    return *std::min_element(lowerBoundDurations.begin(), lowerBoundDurations.end());
  }

  static inline std::ostream &operator<<(std::ostream &os, const std::vector<Constraint> &guards) {
    bool isFirst = true;
    for (const Constraint &guard: guards) {
      if (!isFirst) {
        os << ", ";
      }
      os << guard;
      isFirst = false;
    }

    return os;
  }

  inline std::vector<std::vector<Constraint>> negate(const std::vector<std::vector<Constraint>> &dnfConstraints) {
    std::vector<std::vector<Constraint>> cnfNegated;
    cnfNegated.reserve(dnfConstraints.size());
    // Negation as CNF
    std::transform(dnfConstraints.begin(), dnfConstraints.end(), std::back_inserter(cnfNegated), negateAll);
    std::vector<std::vector<Constraint>> dnfNegated;
    // Transform CNF to DNF
    bool initial = true;
    for (const auto &disjunct: cnfNegated) {
      if (disjunct.empty()) {
        continue;
      }
      if (initial) {
        dnfNegated.reserve(disjunct.size());
        std::transform(disjunct.begin(), disjunct.end(), std::back_inserter(dnfNegated), [](const auto &constraint) {
          return std::vector<Constraint>{constraint};
        });
        initial = false;
      } else {
        std::vector<std::vector<Constraint>> newDnfNegated;
        for (const auto &constraint: disjunct) {
          for (auto conjunct: dnfNegated) {
            conjunct.push_back(constraint);
            if (satisfiable(conjunct)) {
              newDnfNegated.push_back(simplify(conjunct));
            }
          }
        }
        for (auto it = newDnfNegated.begin(); it != newDnfNegated.end();) {
          if (std::any_of(it + 1, newDnfNegated.end(), [&](const auto &constraints) {
            if (isWeaker(constraints, *it)) {
              BOOST_LOG_TRIVIAL(trace) << constraints << " is weaker than " << *it;
            }
            return isWeaker(constraints, *it);
          }) || std::any_of(newDnfNegated.begin(), it, [&](const auto &constraints) {
            if (isWeaker(constraints, *it)) {
              BOOST_LOG_TRIVIAL(trace) << constraints << " is weaker than " << *it;
            }
            return isWeaker(constraints, *it);
          })) {
            it = newDnfNegated.erase(it);
          } else {
            ++it;
          }
        }
        dnfNegated = std::move(newDnfNegated);
      }
    }

    return dnfNegated;
  }

  /*!
   * @brief Return the strongest guard that is weaker than all of the given guards
   */
  static inline auto unionHull(const std::vector<std::vector<Constraint>> &guards) {
    boost::unordered_map<std::pair<ClockVariables, bool>, Bounds> guardsAsBounds;
    bool initial = true;
    for (const auto& guard: guards) {
      boost::unordered_set<std::pair<ClockVariables, bool>> boundedKeys;
      for (const auto &constraint: guard) {
        const auto clock = constraint.x;
        const auto upperBound = constraint.isUpperBound();
        boundedKeys.emplace(clock, upperBound);
        auto it = guardsAsBounds.find(std::make_pair(clock, upperBound));
        if (it == guardsAsBounds.end()) {
          if (initial) {
            guardsAsBounds[std::make_pair(clock, upperBound)] = constraint.toDBMBound();
          }
        } else {
          it->second = std::max(it->second, constraint.toDBMBound());
        }
      }
      if (boundedKeys.size() < guardsAsBounds.size()) {
        // We have unbounded variables
        for (auto it = guardsAsBounds.begin(); it!= guardsAsBounds.end(); ) {
          if (boundedKeys.find(it->first) == boundedKeys.end()) {
            it = guardsAsBounds.erase(it);
          } else {
            ++it;
          }
        }
      }
      initial = false;
    }

    std::vector<Constraint> result;
    result.reserve(guardsAsBounds.size());
    std::transform(guardsAsBounds.begin(), guardsAsBounds.end(), std::back_inserter(result), [] (const auto p) {
      const auto clock = p.first.first;
      const auto upperBound = p.first.second;
      const auto bound = p.second;
      if (upperBound) {
        if (bound.second) {
          return ConstraintMaker(clock) <= bound.first;
        } else {
          return ConstraintMaker(clock) < bound.first;
        }
      } else {
        if (bound.second) {
          return ConstraintMaker(clock) >= -bound.first;
        } else {
          return ConstraintMaker(clock) > -bound.first;
        }
      }
    });

    // Assert the weakness
    assert(std::all_of(guards.begin(), guards.end(), [&] (const auto guard) {
      return isWeaker(result, guard);
    }));

    return result;
  }

  /*!
   * @brief Return the strongest guard that is weaker than the given guards
   */
  static inline auto unionHull(const std::vector<Constraint> &left, const std::vector<Constraint> &right) {
    return unionHull(std::vector<std::vector<Constraint>>{left, right});
  }

  inline void addUpperBound(std::vector<Constraint> &guard) {
    std::unordered_map<ClockVariables, std::vector<Constraint>> mapFromClock;
    for (const auto &constraint: guard) {
      auto it = mapFromClock.find(constraint.x);
      if (it == mapFromClock.end()) {
        mapFromClock[constraint.x] = {constraint};
      } else {
        mapFromClock.at(constraint.x).push_back(constraint);
      }
    }
    for (const auto&[clock, constraints]: mapFromClock) {
      // assume that the guard is non-redundant
      if (constraints.size() == 1 && !constraints.front().isUpperBound()) {
        if (constraints.front().odr == Constraint::Order::le) {
          guard.push_back(ConstraintMaker(clock) <= constraints.front().c);
        } else {
          guard.push_back(ConstraintMaker(clock) < constraints.front().c + 1);
        }
      }
    }
  }
}
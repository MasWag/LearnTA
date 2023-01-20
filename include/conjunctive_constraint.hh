/**
 * @author Masaki Waga
 * @date 2023/01/20.
 */

#pragma once

#include "zone.hh"
#include "constraint.hh"

namespace learnta {
#ifndef NO_EIGEN_CONSTRAINT
  /*!
   * @brief Conjunction of inequality constraints represented by a vector
   *
   * The encoding is much like DBM but it is a vector since we do not have diagonal constraints
   */
  class ConjunctiveInequalities {
  private:
    //! The number of variables
    Eigen::Index dimension;
    //! The actual container
    Eigen::VectorX<IntBounds> value;
    explicit ConjunctiveInequalities(Eigen::VectorX<IntBounds> &&value):
              dimension (value.size() / 2), value(value) {}
  public:
    // We do not use std::numeric_limits<int>::max() to avoid overflow
    int maxValue = std::numeric_limits<int>::max() / 2;
    explicit ConjunctiveInequalities(Eigen::Index dimension) : dimension(dimension) {
      assert(dimension > 0);
      value.resize(dimension * 2);
      value.head(dimension).fill(IntBounds{maxValue, false});
      value.tail(dimension).fill(IntBounds{0, true});
    }

    ConjunctiveInequalities(const Constraint &constraint, Eigen::Index dimension) : dimension(dimension) {
      assert(dimension > 0);
      value.resize(dimension * 2);
      value.head(dimension).fill(IntBounds{maxValue, false});
      value.tail(dimension).fill(IntBounds{0, true});
      assert(constraint.x < dimension);
      const IntBounds bound = constraint.toBound();
      if (constraint.isUpperBound()) {
        value[constraint.x] = bound;
      } else {
        value[constraint.x + dimension] = static_cast<IntBounds>(-bound);
      }
    }

    ConjunctiveInequalities(const std::vector<Constraint> &constraints, Eigen::Index dimension) : dimension(dimension) {
      assert(dimension > 0);
      value.resize(dimension * 2);
      value.head(dimension).fill(IntBounds{maxValue, false});
      value.tail(dimension).fill(IntBounds{0, true});
      for (const auto& constraint: constraints) {
        assert(constraint.x < dimension);
        const IntBounds bound = constraint.toBound();
        if (constraint.isUpperBound()) {
          value[constraint.x] = bound;
        } else {
          value[constraint.x + dimension] = static_cast<IntBounds>(-bound);
        }
      }
    }

    void addConstraint(const Constraint &constraint) {
      assert(constraint.x < dimension);
       const IntBounds bound = constraint.toBound();
       if (constraint.isUpperBound()) {
         value[constraint.x] = std::min(value[constraint.x], bound);
       } else {
         value[constraint.x + dimension] = std::min(value[constraint.x + dimension], static_cast<IntBounds>(-bound));
       }
    }

    /*!
     * @brief Returns the intersection of two zones
     */
    ConjunctiveInequalities operator&&(const ConjunctiveInequalities &another) const {
      assert(this->value.cols() == another.value.cols());
      assert(this->value.rows() == another.value.rows());
      auto result = ConjunctiveInequalities{this->value.array().min(another.value.array())};

      return result;
    }

    /*!
     * @brief Assign the intersection of two zones
     */
    ConjunctiveInequalities operator&=(const ConjunctiveInequalities &another) {
      assert(this->value.cols() == another.value.cols());
      assert(this->value.rows() == another.value.rows());
      this->value = this->value.cwiseMin(another.value);

      return *this;
    }

    /*!
     * @brief Check if the constraint is satisfiable
     */
    [[nodiscard]] bool isSatisfiable() const {
      return (value.head(dimension) + value.tail(dimension)).minCoeff() >= IntBounds(0, true);
    }

    /*!
     * @brief check if the constraint is satisfiable
     */
    explicit operator bool() const {
      return isSatisfiable();
    }

    /*!
     * @brief check if the constraint is satisfiable
     */
    [[nodiscard]] std::vector<Constraint> asConstraints() const {
      std::vector<Constraint> result;
      result.reserve(dimension * 2);
      for (int i = 0; i < dimension; ++i) {
        if (value[i].first >= this->maxValue) {
          continue;
        } else if (value[i].second) {
          result.push_back(ConstraintMaker(i) <= value[i].first);
        } else {
          result.push_back(ConstraintMaker(i) < value[i].first);
        }
      }
      for (int i = 0; i < dimension; ++i) {
        if (value[dimension + i] >= IntBounds{0, true}) {
          continue;
        } else if (value[dimension + i].second) {
          result.push_back(ConstraintMaker(i) >= -value[dimension + i].first);
        } else {
          result.push_back(ConstraintMaker(i) > -value[dimension + i].first);
        }
      }
      return result;
    }

    /*!
     * @brief Return if this constraint includes the given constraint
     */
    [[nodiscard]] bool includes(const ConjunctiveInequalities &guard) const {
      return this->value.cwiseMax(guard.value) == this->value;
    };

    /*!
     * @brief Check the equivalence of two constraints
     */
    bool operator==(const ConjunctiveInequalities &guard) const {
      return value == guard.value;
    }
  };

  inline std::vector<std::vector<Constraint>> negate(const std::vector<std::vector<Constraint>> &dnfConstraints) {
    Eigen::Index dimension = 0;
    for (const auto& disjunct: dnfConstraints) {
      for (const auto& constraint: disjunct) {
        dimension = std::max(dimension, 1 + static_cast<Eigen::Index>(constraint.x));
      }
    }
    std::vector<std::vector<Constraint>> cnfNegated;
    cnfNegated.reserve(dnfConstraints.size());
    // Negation as CNF
    std::transform(dnfConstraints.begin(), dnfConstraints.end(), std::back_inserter(cnfNegated), negateAll);
    std::vector<ConjunctiveInequalities> dnfNegated;
    // Transform CNF to DNF
    bool initial = true;
    for (const auto &disjunct: cnfNegated) {
      if (disjunct.empty()) {
        continue;
      }
      if (initial) {
        dnfNegated.reserve(disjunct.size());
        std::transform(disjunct.begin(), disjunct.end(), std::back_inserter(dnfNegated), [&](const auto &constraint) {
          return ConjunctiveInequalities{constraint, dimension};
        });
        initial = false;
      } else {
        std::vector<ConjunctiveInequalities> newDnfNegated;
        for (const auto &constraint: disjunct) {
          for (auto conjunct: dnfNegated) {
            conjunct.addConstraint(constraint);
            if (conjunct) {
              newDnfNegated.push_back(conjunct);
            }
          }
        }
        for (auto it = newDnfNegated.begin(); it != newDnfNegated.end();) {
          if (std::any_of(it + 1, newDnfNegated.end(), [&](const ConjunctiveInequalities &constraints) {
            return constraints.includes(*it);
          }) || std::any_of(newDnfNegated.begin(), it, [&](const ConjunctiveInequalities &constraints) {
            return constraints.includes(*it);
          })) {
            it = newDnfNegated.erase(it);
          } else {
            ++it;
          }
        }
        dnfNegated = std::move(newDnfNegated);
      }
    }

    std::vector<std::vector<Constraint>> result;
    result.reserve(dnfNegated.size());
    std::transform(dnfNegated.begin(), dnfNegated.end(), std::back_inserter(result),
                   std::mem_fn(&ConjunctiveInequalities::asConstraints));
    return result;
  }
#endif
}
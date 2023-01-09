/**
 * @author Masaki Waga
 * @date 2022/03/21.
 */

#pragma once

#include <vector>
#include <unordered_set>
#include <utility>
#include <ostream>

#include "timed_condition.hh"

namespace learnta {
  class RenamingRelation : public std::vector<std::pair<std::size_t, std::size_t>> {
  public:
    /*!
     * @brief Construct a valuation from a timed condition
     *
     * @pre condition is simple
     */
    static std::vector<double> toValuation(TimedCondition condition) {
      std::vector<double> result;
      result.resize(condition.size());
      for (std::size_t i = 0; i < condition.size(); ++i) {
        const auto lowerBound = condition.getLowerBound(i, condition.size() - 1);
        const auto upperBound = condition.getUpperBound(i, condition.size() - 1);
        if (lowerBound.first == -upperBound.first && lowerBound.second && upperBound.second) {
          // When the bound is a point
          result.at(i) = upperBound.first;
        } else {
          // When the bound is not a point
          auto middlePoint = (upperBound.first - lowerBound.first) / 2.0;
          result.at(i) = middlePoint;
          condition.restrictLowerBound(i, condition.size() - 1, Bounds{-middlePoint, true}, false);
          condition.restrictUpperBound(i, condition.size() - 1, Bounds{middlePoint, true}, false);
        }
      }

      return result;
    }

    [[nodiscard]] std::vector<std::pair<ClockVariables, std::variant<double, ClockVariables>>>
    toReset(const TimedCondition &sourceCondition, const TimedCondition &targetCondition) const {
      // Construct the reset from the renaming relation
      std::vector<std::pair<ClockVariables, std::variant<double, ClockVariables>>> result;
      result.resize(this->size());
      std::transform(this->begin(), this->end(), result.begin(), [&](const auto &renamingPair) {
        return std::make_pair(renamingPair.second, static_cast<ClockVariables>(renamingPair.first));
      });

      // Construct the reset from the timed conditions
      const auto targetValuation = toValuation(targetCondition);
      // Map the clock variables to the target timed condition if it is not mapped with the renaming relation
      for (std::size_t resetVariable = 0; resetVariable < targetCondition.size(); ++resetVariable) {
        auto it = std::find_if(result.begin(), result.end(), [&](const auto &pair) {
          return pair.first == resetVariable;
        });
        if (it == result.end()) {
          result.emplace_back(resetVariable, targetValuation.at(resetVariable));
        } else if (targetValuation.at(resetVariable) == std::floor(targetValuation.at(resetVariable))) {
          // We overwrite the assigned value if it is a constant
          it->second = targetValuation.at(resetVariable);
        }
      }

      return result;
    }

    template<class T>
    std::vector<T> apply(const std::vector<T> &value) const {
      assert(std::all_of(this->begin(), this->end(), [&value] (const auto pair) {
        return value.size() > pair.first;
      }));
      std::vector<T> result = value;
      for (const auto &[source, target]: *this) {
        if (result.size() <= target) {
          result.resize(target);
        }
        result.at(target) = result.at(source);
      }

      return result;
    }

    /*!
     * @brief The clock variables on the right hand side of the renaming relation
     */
    [[nodiscard]] auto rightVariables() const {
      std::vector<ClockVariables> result;
      result.reserve(this->size());
      std::transform(this->begin(), this->end(), std::back_inserter(result), [] (const auto &pair) {
        return pair.second;
      });
      // Assert that the right variables have no duplications
      assert(result.size() == std::unordered_set<size_t>(result.begin(), result.end()).size());
      return result;
    }

    /*!
     * @brief Erase pairs such that (left, right) for some right
     */
     void eraseLeft(std::size_t left) {
       this->erase(std::remove_if(this->begin(), this->end(), [&] (const auto &pair) {
         return pair.first == left;
       }), this->end());
    }

    /*!
     * @brief Check if the renaming relation contains only trivial equation from the timed conditions
     */
    [[nodiscard]] bool onlyTrivial(const TimedCondition &sourceCondition, const TimedCondition &targetCondition) const {
      const auto juxtaposedCondition = sourceCondition ^ targetCondition;

      return std::all_of(this->begin(), this->end(), [&] (const auto &renamingPair) {
        const auto &[sourceClock, targetClock] = renamingPair;
        const auto sourceUpperBound = sourceCondition.getUpperBound(sourceClock, sourceCondition.size() - 1);
        const auto sourceLowerBound = sourceCondition.getLowerBound(sourceClock, sourceCondition.size() - 1);
        const auto targetUpperBound = targetCondition.getUpperBound(targetClock, targetCondition.size() - 1);
        const auto targetLowerBound = targetCondition.getLowerBound(targetClock, targetCondition.size() - 1);

        return isPoint(sourceUpperBound, sourceLowerBound) && isPoint(targetUpperBound, targetLowerBound) &&
               sourceUpperBound == targetUpperBound && sourceLowerBound == targetLowerBound;
      });
    }

    /*!
     * @brief Check if the renaming relation is full, i.e., all the right variables are bounded
     */
    [[nodiscard]] bool full(const TimedCondition &condition) const {
      return this->size() == condition.size();
    }

    /*!
     * @brief Check if the application of this renaming causes implicit clocks
     */
    [[nodiscard]] bool impreciseClocks(const TimedCondition &source, const TimedCondition &target) const {
      return !this->empty() && !this->full(target) && !this->onlyTrivial(source, target);
    }

    /*!
     * @brief Add renaming equations from target and source conditions
     */
    void addImplicitConstraints(JuxtaposedZone juxtaposedCondition) {
      juxtaposedCondition.addRenaming(*this);
      auto newRenaming = juxtaposedCondition.makeRenaming();
      std::move(newRenaming.begin(), newRenaming.end(), std::back_inserter(*this));
      // Make it unique in terms of the right variables
      std::sort(this->begin(), this->end(), [] (const auto &left, const auto& right) {
        return left.second < right.second || (left.second == right.second && left.first < right.first);
      });
      this->erase(std::unique(this->begin(), this->end(), [] (const auto &left, const auto &right) {
        return left.second == right.second;
      }), this->end());
    }

    /*!
     * @brief Add renaming equations from target and source conditions
     */
     void addImplicitConstraints(const TimedCondition &source, const TimedCondition &target) {
      addImplicitConstraints(source ^ target);
     }

    friend std::ostream &operator<<(std::ostream &os, const RenamingRelation &relation) {
      bool isFirst = true;
      os << '{';
      for (const auto & pair: relation) {
        if (!isFirst) {
          os << " && ";
        }
        os << "t" << pair.first << " == t'" << pair.second;
        isFirst = false;
      }
      os << '}';
      return os;
    }
  };
}

namespace std {
  template<class T, class U>
  std::ostream &operator<<(std::ostream &os, const std::pair<T, U> &pair) {
    os << '{' << pair.first << ", " << pair.second << "}";

    return os;
  }
}
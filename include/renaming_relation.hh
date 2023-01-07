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
    // TODO: Probably we will have to change this function.
    [[nodiscard]] std::vector<std::pair<ClockVariables, std::variant<double, ClockVariables>>>
    toReset(const TimedCondition &sourceCondition, const TimedCondition &targetCondition) const {
      // Construct the reset from the renaming relation
      std::vector<std::pair<ClockVariables, std::variant<double, ClockVariables>>> result;
      result.resize(this->size());
      std::transform(this->begin(), this->end(), result.begin(), [&](const auto &renamingPair) {
        return std::make_pair(renamingPair.second, static_cast<ClockVariables>(renamingPair.first));
      });

      // Construct the reset from the timed conditions
      // TODO: I am not too sure if we need this part
      std::size_t j = 0;
      for (std::size_t i = 0; i < targetCondition.size(); ++i) {
        if (std::find_if(result.begin(), result.end(), [&](const auto p) { return p.first == i; }) != result.end()) {
          continue;
        }
        if (j >= sourceCondition.size()) {
          return result;
        }
        while (targetCondition.getUpperBound(i, targetCondition.size() - 1) !=
               sourceCondition.getUpperBound(j, sourceCondition.size() - 1) ||
               targetCondition.getLowerBound(i, targetCondition.size() - 1) !=
               sourceCondition.getLowerBound(j, sourceCondition.size() - 1) ||
               // The value of j should not be modified by the transition
               (j < targetCondition.size() &&
                targetCondition.getUpperBound(j, targetCondition.size() - 1) !=
                sourceCondition.getUpperBound(j, sourceCondition.size() - 1)) ||
               (j < targetCondition.size() &&
                targetCondition.getLowerBound(j, targetCondition.size() - 1) !=
                sourceCondition.getLowerBound(j, sourceCondition.size() - 1))) {
          j++;
          if (j >= sourceCondition.size()) {
            return result;
          }
        }
        if (i != j) {
          result.emplace_back(i, static_cast<ClockVariables>(j));
        }
        if (i + 1 < targetCondition.size() && targetCondition.getLowerBound(i, i + 1) != Bounds{0, true}) {
          j++;
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
     * @brief Check if the renaming relation contains only trivial equation from the timed conditions
     */
    [[nodiscard]] bool onlyTrivial(const TimedCondition &sourceCondition, const TimedCondition &targetCondition) const {
      const auto juxtaposedCondition = sourceCondition ^ targetCondition;

      return std::all_of(this->begin(), this->end(), [&juxtaposedCondition] (const auto &renamingPair) {
        const auto &[sourceClock, targetClock] = renamingPair;
        const auto leftIndex = sourceClock + 1;
        const auto rightIndex = targetClock + juxtaposedCondition.getLeftSize() + 1;
        return juxtaposedCondition.value(leftIndex, rightIndex) == Bounds{0, true} &&
               juxtaposedCondition.value(rightIndex, leftIndex) == Bounds{0, true};
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
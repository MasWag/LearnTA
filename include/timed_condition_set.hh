/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include <vector>
#include <utility>

#include "timed_condition.hh"

namespace learnta {
  /*!
   * @brief A set of timed conditions to represent non-convex constraints
   */
  class TimedConditionSet {
  private:
    std::vector<TimedCondition> conditions;

    explicit TimedConditionSet(std::vector<TimedCondition> conditions) : conditions(std::move(conditions)) {}

  public:
    explicit TimedConditionSet(TimedCondition condition) : conditions({std::move(condition)}) {
    }

    static TimedConditionSet bottom() {
      return TimedConditionSet{std::vector<TimedCondition>{}};
    }

    /*!
     * @brief Construct a timed condition set from a set of simple elementary languages
     *
     * @pre All the words should be the same
     * @pre elementaryLanguages is a list of simple elementary languages
     */
    static TimedConditionSet reduce(std::list<ElementaryLanguage> elementaryLanguages) {
      if (elementaryLanguages.empty()) {
        return TimedConditionSet::bottom();
      }
      // Assert the equivalence of all the words
      assert(std::all_of(elementaryLanguages.begin(), elementaryLanguages.end(), [&](const auto &elem) {
        return elem.word == elementaryLanguages.front().word;
      }));
      // Assert the simplicity
      assert(std::all_of(elementaryLanguages.begin(), elementaryLanguages.end(), [&](const auto &elem) {
        return elem.timedCondition.isSimple();
      }));
      std::list<std::pair<TimedCondition, int>> timedConditionsWithSize;
      timedConditionsWithSize.resize(elementaryLanguages.size());
      std::transform(std::make_move_iterator(elementaryLanguages.begin()),
                     std::make_move_iterator(elementaryLanguages.end()),
                     timedConditionsWithSize.begin(),
                     [&](auto &&elementary) {
                       return std::make_pair(elementary.timedCondition, 1);
                     });
      auto it = timedConditionsWithSize.begin();
      while (it != timedConditionsWithSize.end()) {
        auto timedCondition = it->first;
        bool merged = false;
        for (auto it2 = std::next(it); it2 != timedConditionsWithSize.end(); it2++) {
          // Check if the convex hull is the exact union
          auto convexHull = timedCondition.convexHull(it2->first);
          std::vector<TimedCondition> enumerated;
          convexHull.enumerate(enumerated);
          if (enumerated.size() == it->second + it2->second) {
            it->first = std::move(convexHull);
            it->second += it2->second;
            timedConditionsWithSize.erase(it2);
            it = timedConditionsWithSize.begin();
            merged = true;
            break;
          }
        }
        if (!merged) {
          it++;
        }
      }
      std::vector<TimedCondition> result;
      result.resize(timedConditionsWithSize.size());
      std::transform(std::make_move_iterator(timedConditionsWithSize.begin()),
                     std::make_move_iterator(timedConditionsWithSize.end()),
                     result.begin(),
                     [](auto &&timedConditionWithSize) {
                       return timedConditionWithSize.first;
                     });

      return TimedConditionSet{result};
    }

    [[nodiscard]] bool empty() const {
      return this->conditions.empty();
    }

    [[nodiscard]] std::size_t size() const {
      return this->conditions.size();
    }

    [[nodiscard]] TimedCondition front() const {
      return this->conditions.front();
    }

    [[nodiscard]] const std::vector<TimedCondition> &getConditions() const {
      return conditions;
    }

    /*!
     * @brief Returns the set of variables strictly constrained compared with the original condition.
     *
     * @pre this conditions and original condition should have the same variable space.
     */
    [[nodiscard]] std::vector<std::size_t> getStrictlyConstrainedVariables(const TimedCondition &originalCondition,
                                                                           const size_t examinedVariableSize) const {
      std::vector<std::size_t> result;
      for (const auto &condition: this->conditions) {
        std::vector<std::size_t> tmp =
                condition.getStrictlyConstrainedVariables(originalCondition, examinedVariableSize);
        result.reserve(result.size() + tmp.size());
        std::move(tmp.begin(), tmp.end(), std::back_inserter(result));
      }
      return result;
    }
  };
}
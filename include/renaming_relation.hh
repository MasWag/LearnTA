/**
 * @author Masaki Waga
 * @date 2022/03/21.
 */

#pragma once

#include <vector>
#include <utility>

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
      result.erase(std::remove_if(result.begin(), result.end(), [](const auto &resetPair) {
        return resetPair.second.index() == 1 && resetPair.first == std::get<ClockVariables>(resetPair.second);
      }), result.end());

      // Construct the reset from the timed conditions
      int j = 0;
      for (int i = 0; i < targetCondition.size(); ++i) {
        if (std::find_if(result.begin(), result.end(), [&](const auto p) { return p.first == i; }) != result.end()) {
          continue;
        }
        if (j >= sourceCondition.size()) {
          return result;
        }
        while (targetCondition.getUpperBound(i, targetCondition.size() - 1) !=
               sourceCondition.getUpperBound(j, sourceCondition.size() - 1) ||
               targetCondition.getLowerBound(i, targetCondition.size() - 1) !=
               sourceCondition.getLowerBound(j, sourceCondition.size() - 1)) {
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
  };
}
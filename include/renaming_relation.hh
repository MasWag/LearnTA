/**
 * @author Masaki Waga
 * @date 2022/03/21.
 */

#pragma once

#include <vector>
#include <utility>

namespace learnta {
  class RenamingRelation : public std::vector<std::pair<std::size_t, std::size_t>> {
  public:
    [[nodiscard]] std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>> toReset() const {
      std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>> result;
      result.resize(this->size());
      std::transform(this->begin(), this->end(), result.begin(), [&] (const auto &renamingPair) {
        return std::make_pair(renamingPair.first, renamingPair.second);
      });

      return result;
    }
  };
}
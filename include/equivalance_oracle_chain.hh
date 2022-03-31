/**
 * @author Masaki Waga
 * @date 2022/03/31.
 */

#pragma once

#include <vector>

#include "equivalence_oracle.hh"

namespace learnta {
  /*!
   * @brief A chain of the equivalence oracles
   */
  class EquivalenceOracleChain : public EquivalenceOracle {
    std::vector<std::unique_ptr<EquivalenceOracle>> oracles;
  public:
    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) const override {
      for (const auto &oracle: this->oracles) {
        auto result = oracle->findCounterExample(hypothesis);
        if (result) {
          return result;
        }
      }

      return std::nullopt;
    }

    void push_back(std::unique_ptr<EquivalenceOracle> &&oracle) {
      oracles.push_back(std::move(oracle));
    }
  };
}
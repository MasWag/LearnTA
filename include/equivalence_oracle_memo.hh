/**
 * @author Masaki Waga
 * @date 2022/09/20.
 */

#pragma once

#include "equivalence.hh"
#include "equivalence_oracle_by_test.hh"

namespace learnta {
  /*!
   * @brief Memoization of the equivalence oracle
   */
  class EquivalenceOracleMemo : public EquivalenceOracle {
  private:
    std::unique_ptr<EquivalenceOracle> oracle;
    EquivalenceOracleByTest oracleByTest;
  public:
    EquivalenceOracleMemo (std::unique_ptr<EquivalenceOracle> &&oracle, const TimedAutomaton &target) : oracle(std::move(oracle)), oracleByTest(target) {}

    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) override {
      auto result = oracleByTest.findCounterExample(hypothesis);
      if (result) {
        return result;
      } else {
        result = oracle->findCounterExample(hypothesis);
        if (result) {
          oracleByTest.push_back(*result);
        }

        return result;
      }
    }
  };
}
/**
 * @author Masaki Waga
 * @date 2022/09/20.
 */

#pragma once

#include "equivalence.hh"
#include "equivalence_oracle_by_test.hh"

namespace learnta {
  /*!
   * @brief Wrapper of an equivalence oracle to cache the queries.
   *
   * This class memorizes all the previous counterexamples returned by the wrapped equivalence oracle.
   * The memorized inputs are used to check the equivalence by testing before using the actual equivalence oracle.
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
      ++eqQueryCount;
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

    //! @brief Print the statistics
    std::ostream &printStatistics(std::ostream &stream) const override {
      stream << "Number of equivalence queries: " << this->numEqQueries() << "\n";
      stream << "Number of equivalence queries (with cache): " << this->oracle->numEqQueries() << "\n";

      return stream;
    }
  };
}
/**
 * @author Masaki Waga
 * @date 2022/03/20.
 */

#pragma once

#include <memory>

#include "equivalence_oracle.hh"
#include "symbolic_membership_oracle.hh"
#include "observation_table.hh"

namespace learnta {
  /*!
   * @brief The deterministic timed automata learner
   */
  class Learner {
  private:
    std::unique_ptr<EquivalenceOracle> eqOracle;
    ObservationTable observationTable;
    std::size_t eqQueryCount = 0;
  public:
    Learner(const std::vector<Alphabet> &alphabet,
            std::unique_ptr<SymbolicMembershipOracle> memOracle,
            std::unique_ptr<EquivalenceOracle> eqOracle) : eqOracle(std::move(eqOracle)),
                                                           observationTable(alphabet, std::move(memOracle)) {}

    TimedAutomaton run() {
      while (true) {
        bool notUpdated = false;
        do {
          notUpdated = observationTable.close();
          notUpdated = notUpdated && observationTable.consistent();
          notUpdated = notUpdated && observationTable.exteriorConsistent();
        } while (notUpdated);
        auto hypothesis = observationTable.generateHypothesis();
        const auto counterExample = eqOracle->findCounterExample(hypothesis);
        eqQueryCount++;

        if (counterExample) {
          observationTable.addCounterExample(ForwardRegionalElementaryLanguage::fromTimedWord(counterExample.value()));
        } else {
          return hypothesis;
        }
      }
    }

    std::ostream &printStatistics(std::ostream &stream) {
      this->observationTable.printStatistics(stream);

      stream << "Number of equivalence queries: " << this->eqQueryCount << "\n";

      return stream;
    }
  };
}
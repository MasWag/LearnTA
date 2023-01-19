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
  public:
    Learner(const std::vector<Alphabet> &alphabet,
            std::unique_ptr<SymbolicMembershipOracle> memOracle,
            std::unique_ptr<EquivalenceOracle> eqOracle) : eqOracle(std::move(eqOracle)),
                                                           observationTable(alphabet, std::move(memOracle)) {}

    TimedAutomaton run() {
      while (true) {
        bool notUpdated;
        do {
          notUpdated = observationTable.close();
          notUpdated = notUpdated && observationTable.consistent();
          notUpdated = notUpdated && observationTable.exteriorConsistent();
          notUpdated = notUpdated && observationTable.timeSaturate();
          // notUpdated = notUpdated && observationTable.renameConsistent();
        } while (!notUpdated);
        BOOST_LOG_TRIVIAL(debug) << "Start DTA generation";
        auto hypothesis = observationTable.generateHypothesis();
        BOOST_LOG_TRIVIAL(debug) << "Hypothesis before simplification\n" << hypothesis;
        hypothesis.simplifyStrong();
        BOOST_LOG_TRIVIAL(debug) << "Hypothesis before zone-based simplification\n" << hypothesis;
        hypothesis.simplifyWithZones();
        BOOST_LOG_TRIVIAL(info) << "The learner generated a hypothesis\n" << hypothesis;
        assert(hypothesis.deterministic());
        const auto counterExample = eqOracle->findCounterExample(hypothesis);

        if (counterExample) {
          BOOST_LOG_TRIVIAL(info) << "Equivalence oracle returned a counter example: " << counterExample.value();
          observationTable.handleCEX(counterExample.value());
        } else {
          return hypothesis;
        }
      }
    }

    std::ostream &printStatistics(std::ostream &stream) const {
      this->observationTable.printStatistics(stream);
      this->eqOracle->printStatistics(stream);

      return stream;
    }

    [[nodiscard]] std::size_t numEqQueries() const {
      return this->eqOracle->numEqQueries();
    }
  };
}
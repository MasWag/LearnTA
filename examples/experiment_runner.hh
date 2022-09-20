/**
 * @author Masaki Waga
 * @date 2022/09/09.
 */

#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <utility>

#include "timed_automaton.hh"
#include "sul.hh"
#include "timed_automaton_runner.hh"
#include "symbolic_membership_oracle.hh"
#include "equivalence_oracle_by_test.hh"
#include "timed_automata_equivalence_oracle.hh"
#include "learner.hh"
#include "equivalance_oracle_chain.hh"
#include "equivalence_oracle_memo.hh"

namespace learnta {

  class ExperimentRunner {
  private:
    const std::vector<Alphabet> alphabet;
    TimedAutomaton target;
    std::vector<TimedWord> testWords;
  public:

    void pushTestWord(const TimedWord& testWord) {
      testWords.push_back(testWord);
    }

    ExperimentRunner(std::vector<Alphabet> alphabet, TimedAutomaton target) : alphabet(std::move(alphabet)), target(std::move(target)) {}

    /*!
     * @brief Execute the experiment
     */
    void run() const {
      BOOST_LOG_TRIVIAL(info) << "Target DTA\n" << this->target;
      TimedAutomaton complement = this->target.complement(this->alphabet);
      complement.simplifyStrong();
      complement.simplifyWithZones();
      BOOST_LOG_TRIVIAL(info) << "Complement of the target DTA\n" << complement;

      // Construct the learner
      auto sul = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner(this->target));
      auto memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(sul));
      auto eqOracle = std::make_unique<learnta::EquivalenceOracleChain>();
      auto eqOracleByTest = std::make_unique<learnta::EquivalenceOracleByTest>(this->target);
      // Equivalence query by static string to make the evaluation stable
      // These strings are generated in macOS but the equivalence query returns a different set of counter examples.
      // This deviation is probably because of the difference in the address handling, which is used in sorting.
      for (const auto& testWord: this->testWords) {
        eqOracleByTest->push_back(testWord);
      }

      eqOracle->push_back(std::move(eqOracleByTest));
      eqOracle->push_back(
              std::make_unique<learnta::ComplementTimedAutomataEquivalenceOracle>(
                      this->target, complement, alphabet));
      learnta::Learner learner{alphabet, std::move(memOracle),
                               std::make_unique<learnta::EquivalenceOracleMemo>(std::move(eqOracle), this->target)};

      // Run the learning
      BOOST_LOG_TRIVIAL(info) << "Start Learning!!";
      const auto startTime = std::chrono::system_clock::now(); // Current time
      const auto hypothesis = learner.run();
      const auto endTime = std::chrono::system_clock::now(); // End time

      BOOST_LOG_TRIVIAL(info) << "Learning Finished!!";
      BOOST_LOG_TRIVIAL(info) << "The learned DTA is as follows\n" << hypothesis;
      learner.printStatistics(std::cout);
      BOOST_LOG_TRIVIAL(info) << "Execution Time: "
                              << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()
                              << " [ms]";
    }
  };
}
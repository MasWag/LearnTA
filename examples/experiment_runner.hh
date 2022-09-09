/**
 * @author Masaki Waga
 * @date 2022/09/09.
 */

#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

#include "timed_automaton.hh"

namespace learnta {

  class ExperimentRunner {
  private:
    const std::vector<Alphabet> alphabet;
    TimedAutomaton target;
    std::vector<TimedWord> testWords;

    void pushTestWord(const TimedWord& testWord) {
      testWords.push_back(testWord);
    }

  public:
    ExperimentRunner(const std::vector<Alphabet> &alphabet, const TimedAutomaton &target) : alphabet(alphabet),
                                                                                            target(target) {}

  private:
    /*!
     * @brief Execute the experiment
     */
    void run() const {
      BOOST_LOG_TRIVIAL(info) << "Target DTA\n" << this->target;
      TimedAutomaton complement = this->target.complement(this->alphabet);
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
                      this->target, this->target, alphabet));
      learnta::Learner learner{alphabet, std::move(memOracle), std::move(eqOracle)};

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
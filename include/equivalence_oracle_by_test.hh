/**
 * @author Masaki Waga
 * @date 2022/03/15.
 */

#pragma once

#include <optional>
#include <utility>

#include "timed_automaton.hh"
#include "timed_word.hh"
#include "equivalence_oracle.hh"
#include "timed_automaton_runner.hh"

namespace learnta {
  /*!
   * @brief Interface of the equivalence oracle
   */
  class EquivalenceOracleByTest : public EquivalenceOracle {
    std::vector<TimedWord> words;
    TimedAutomaton automaton;
  public:
    explicit EquivalenceOracleByTest(TimedAutomaton automaton) : automaton(std::move(automaton)) {}

    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) override {
      TimedAutomatonRunner runner(automaton);
      TimedAutomatonRunner hypothesisRunner(hypothesis);
     for (const auto &word: words) {
       runner.pre();
       hypothesisRunner.pre();
       for (std::size_t i = 0; i < word.wordSize(); ++i) {
         if (runner.step(word.getDurations().at(i)) != hypothesisRunner.step(word.getDurations().at(i))) {
           return word;
         }
         if (runner.step(word.getWord().at(i)) != hypothesisRunner.step(word.getWord().at(i))) {
           return word;
         }
       }
       if (runner.step(word.getDurations().at(word.wordSize())) != hypothesisRunner.step(word.getDurations().at(word.wordSize()))) {
         return word;
       }
     }

     return std::nullopt;
    }

    void push_back(TimedWord word) {
      words.push_back(std::move(word));
    }
  };
}
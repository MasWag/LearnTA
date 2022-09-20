/**
 * @author Masaki Waga
 * @date 2022/03/15.
 */

#pragma once

#include <optional>
#include <utility>
#include <random>

#include "timed_automaton.hh"
#include "timed_word.hh"
#include "equivalence_oracle.hh"
#include "timed_automaton_runner.hh"

namespace learnta {
  /*!
   * @brief The equivalence oracle by random test
   */
  class EquivalenceOracleByRandomTest : public EquivalenceOracle {
    const std::vector<Alphabet> alphabet;
    const TimedAutomaton automaton;
    const int maxTests;
    const int maxLength;
    const double maxDuration;
  public:
    EquivalenceOracleByRandomTest(std::vector<Alphabet> alphabet, TimedAutomaton automaton,
                                  const int maxTests, const int maxLength, const int maxDuration) :
            alphabet(std::move(alphabet)),
            automaton(std::move(automaton)),
            maxTests(maxTests),
            maxLength(maxLength),
            maxDuration(maxDuration) {}

    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) override {
      static std::random_device rng;
      std::mt19937 engine(rng());

      TimedAutomatonRunner runner(automaton);
      TimedAutomatonRunner hypothesisRunner(hypothesis);
      auto durationDist = std::uniform_real_distribution<double>(0, maxDuration);
      auto actionDist = std::uniform_int_distribution(0, int(alphabet.size() - 1));

      for (int i = 0; i < maxTests; ++i) {
        runner.pre();
        hypothesisRunner.pre();
        std::string word;
        std::vector<double> durations;
        for (int j = 0; j < maxLength; ++j) {
          const double duration = durationDist(engine);
          durations.push_back(duration);
          if (runner.step(duration) != hypothesisRunner.step(duration)) {
            return TimedWord{word, durations};
          }
          const auto wordIndex = actionDist(engine);
          word.push_back(alphabet.at(wordIndex));
          if (runner.step(alphabet.at(wordIndex)) != hypothesisRunner.step(alphabet.at(wordIndex))) {
            durations.push_back(0);
            return TimedWord{word, durations};
          }
        }
        const double duration = durationDist(engine);
        durations.push_back(duration);
        if (runner.step(duration) != hypothesisRunner.step(duration)) {
          return TimedWord{word, durations};
        }

        runner.post();
        hypothesisRunner.post();
      }

      return std::nullopt;
    }
  };
}
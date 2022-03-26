/**
 * @author Masaki Waga
 * @date 2022/03/15.
 */

#pragma once

#include "equivalence_oracle.hh"
#include "intersection.hh"
#include "ta2za.hh"

#include <utility>

namespace learnta {
  /*!
   * @brief Equivalence oracle with a timed automaton recognizing the complement of the target language
   */
  class ComplementTimedAutomataEquivalenceOracle : public EquivalenceOracle {
  private:
    TimedAutomaton target;
    TimedAutomaton complement;

    [[nodiscard]] std::optional<TimedWord> subset(const TimedAutomaton &hypothesis) const {
      TimedAutomaton intersection;
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      intersectionTA(complement, hypothesis, intersection, toIState);
      ZoneAutomaton zoneAutomaton;
      ta2za(intersection, zoneAutomaton);

      return zoneAutomaton.sample();
    }

    [[nodiscard]] std::optional<TimedWord> supset(const TimedAutomaton &hypothesis) const {
      TimedAutomaton intersection;
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      intersectionTA(target, hypothesis.complement(), intersection, toIState);
      ZoneAutomaton zoneAutomaton;
      ta2za(intersection, zoneAutomaton);

      return zoneAutomaton.sample();
    }
      public:
    /*!
     * @param[in] complement A timed automaton recognizing the complement of the target language
     */
    ComplementTimedAutomataEquivalenceOracle(TimedAutomaton target, TimedAutomaton complement) :
            target(std::move(target)), complement(std::move(complement)) {}

    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) const override {
      auto subCounterExample = subset(hypothesis);
      if (subCounterExample) {
        return subCounterExample;
      }

      return supset(hypothesis);
    }
  };
}
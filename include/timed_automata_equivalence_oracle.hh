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
   *
   * @note This is not perfectly reliable because this does not work when the transition of the constructed DTA is not total
   */
  class ComplementTimedAutomataEquivalenceOracle : public EquivalenceOracle {
  private:
    TimedAutomaton target;
    TimedAutomaton complement;
    std::vector<Alphabet> alphabet;

    [[nodiscard]] std::optional<TimedWord> subset(const TimedAutomaton &hypothesis) const {
      TimedAutomaton intersection;
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      intersectionTA(complement, hypothesis, intersection, toIState);
      ZoneAutomaton zoneAutomaton;
      ta2za(intersection.simplify(), zoneAutomaton);

      return zoneAutomaton.sample();
    }

    [[nodiscard]] std::optional<TimedWord> supset(const TimedAutomaton& hypothesis) const {
      TimedAutomaton intersection;
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      intersectionTA(target, hypothesis.complement(this->alphabet), intersection, toIState);
      ZoneAutomaton zoneAutomaton;
      ta2za(intersection.simplify(), zoneAutomaton);

      return zoneAutomaton.sample();
    }

  public:
    /*!
     * @param[in] complement A timed automaton recognizing the complement of the target language
     */
    ComplementTimedAutomataEquivalenceOracle(TimedAutomaton target, TimedAutomaton complement,
                                             std::vector<Alphabet> alphabet) : target(std::move(target)),
                                                                               complement(std::move(complement)),
                                                                               alphabet(std::move(alphabet)) {}

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
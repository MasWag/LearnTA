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
  class ComplementTimedAutomataEquivalenceOracle {
  private:
    TimedAutomaton complement;
  public:
    /*!
     * @param[in] complement A timed automaton recognizing the complement of the target language
     */
    explicit ComplementTimedAutomataEquivalenceOracle(TimedAutomaton complement) : complement(std::move(complement)) {}

    /*!
     * @brief Make an equivalence query
     */
    std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) {
      TimedAutomaton intersection;
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      intersectionTA(complement, hypothesis, intersection, toIState);
      ZoneAutomaton zoneAutomaton;
      ta2za(intersection, zoneAutomaton);

      return zoneAutomaton.sample();
    }
  };
}
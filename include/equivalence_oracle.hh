/**
 * @author Masaki Waga
 * @date 2022/03/15.
 */

#pragma once

#include <optional>

#include "timed_automaton.hh"
#include "timed_word.hh"

namespace learnta {
  /*!
   * @brief Interface of the equivalence oracle
   */
  class EquivalenceOracle {
  public:
    virtual ~EquivalenceOracle() = default;

    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] virtual std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) const = 0;
  };
}
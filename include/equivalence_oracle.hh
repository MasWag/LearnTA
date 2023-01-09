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
  protected:
    std::size_t eqQueryCount = 0;
  public:
    virtual ~EquivalenceOracle() = default;

    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] virtual std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) = 0;

    //! @brief Return the number of the executed equivalence queries
    [[nodiscard]] std::size_t numEqQueries() const {
      return eqQueryCount;
    }

    //! @brief Print the statistics
    virtual std::ostream &printStatistics(std::ostream &stream) const {
      stream << "Number of equivalence queries: " << this->numEqQueries() << "\n";

      return stream;
    }
  };
}
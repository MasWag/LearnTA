#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace learnta {
  typedef char Alphabet;
  typedef uint8_t ClockVariables;
  // Special action for unobservable transitions
  const Alphabet UNOBSERVABLE = 0;
  const auto UNOBSERVABLE_STRING = "ε";

  /*!
   * @brief An automaton
   */
  template<class State>
  struct Automaton {
    struct TATransition;

    //! @brief The states of this automaton.
    std::vector<std::shared_ptr<State>> states;
    //! @brief The initial states of this automaton.
    std::vector<std::shared_ptr<State>> initialStates;

    //! @brief Returns the number of the states.
    [[nodiscard]] inline std::size_t stateSize() const { return states.size(); }

    //! @brief Check the equivalence of two automata
    inline bool operator==(const Automaton<State> &A) const {
      return initialStates == A.initialStates && states == A.states;
    }
  };
}
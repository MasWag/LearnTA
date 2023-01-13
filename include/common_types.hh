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

  /*!
   * @brief Check if the elements are sorted in the ascending order.
   */
  template<class T>
  bool is_ascending(const std::vector<T> &container) {
    static_assert(std::is_arithmetic<T>::value, "The elements must be arithmetic.");
    for (auto it = container.begin(); it != container.end() && std::next(it) != container.end(); ++it) {
      if (*it > *std::next(it)) {
        return false;
      }
    }

    return true;
  }
}
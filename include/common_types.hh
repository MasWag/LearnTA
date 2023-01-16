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

  /*!
   * @brief Check if the elements are sorted in the strict ascending order.
   */
  template<class T>
  bool is_strict_ascending(const std::vector<T> &container) {
    static_assert(std::is_arithmetic<T>::value, "The elements must be arithmetic.");
    for (auto it = container.begin(); it != container.end() && std::next(it) != container.end(); ++it) {
      if (*it >= *std::next(it)) {
        return false;
      }
    }

    return true;
  }

  /*!
   * @brief Return the first element of a pair
   */
  template<class T, class U>
  T first(const std::pair<T, U> &pair) {
    return pair.first;
  }

  /*!
   * @brief Return the second element of a pair
   */
  template<class T, class U>
  U second(const std::pair<T, U> &pair) {
    return pair.second;
  }

  /*!
   * @brief Return a function checking if the first element equal to the given value
   */
  template<class T, class U>
  auto is_first(const T &value) {
    return [&] (const std::pair<T, U> &pair) {
      return pair.first == value;
    };
  }

  /*!
   * @brief Return a function checking if the second element equal to the given value
   */
  template<class T, class U>
  auto is_second(const U &value) {
    return [&] (const std::pair<T, U> &pair) {
      return pair.second == value;
    };
  }
}
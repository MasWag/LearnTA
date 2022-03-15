/**
 * @author Masaki Waga
 * @date 2022/03/15.
 */

#include <vector>
#include <array>
#include <utility>

#include "timed_automaton.hh"

#pragma once
namespace learnta {
  /*!
   * @brief A state of a zone automaton
   */
  struct ZAState {
    //! @brief Shows if this state is accepting
    bool isMatch;
    /*!
     * @brief The transitions from this state
     *
     * @note An epsilon transition is denoted by the null character (\0)
     */
    std::array<std::vector<std::pair<TATransition, std::weak_ptr<ZAState>>>, CHAR_MAX> next;
    TAState *taState = nullptr;
    Zone zone;

    ZAState() : isMatch(false), next({}) {}

    ZAState(TAState *taState, Zone zone) : isMatch(taState->isMatch), next({}), taState(taState),
                                           zone(std::move(zone)) {}

    explicit ZAState(bool isMatch) : isMatch(isMatch), next({}) {}

    ZAState(bool isMatch, std::array<std::vector<std::pair<TATransition, std::weak_ptr<ZAState>>>, CHAR_MAX> next)
            : isMatch(isMatch), next(std::move(next)) {}

    bool operator==(const std::pair<TAState *, Zone> &pair) const {
      return pair.first == taState && pair.second == zone;
    }
  };
}
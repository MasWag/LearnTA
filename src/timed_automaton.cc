/**
 * @author Masaki Waga
 * @date 2022/09/04.
 */

#include <boost/unordered_set.hpp>

#include "timed_automaton.hh"
#include "zone_automaton.hh"
#include "ta2za.hh"

namespace learnta {
  TimedAutomaton learnta::TimedAutomaton::simplifyWithZones() {
    ZoneAutomaton zoneAutomaton;
    ta2za(*this, zoneAutomaton, false);
    zoneAutomaton.removeDeadStates();

    // Make the live states of the TA
    std::unordered_set<TAState *> liveStates;
    std::unordered_map<TAState *, std::vector<std::pair<Alphabet, learnta::TATransition>>> liveTransitions;
    for (const auto &state: zoneAutomaton.states) {
      liveStates.insert(state->taState);
      if (liveTransitions.find(state->taState) == liveTransitions.end()) {
        liveTransitions[state->taState] = {};
      }
      for (int action = 0; action < CHAR_MAX; ++action) {
        const auto &edges = state->next[action];
        for (const auto &[transition, target]: edges) {
          liveTransitions.at(state->taState).emplace_back(action, transition);
        }
      }
    }

    if (liveStates.size() != this->stateSize()) {
      this->states.erase(std::remove_if(this->states.begin(), this->states.end(), [&](const auto &state) {
        return liveStates.find(state.get()) == liveStates.end();
      }), this->states.end());
      this->initialStates.erase(
              std::remove_if(this->initialStates.begin(), this->initialStates.end(), [&](const auto &state) {
                return liveStates.find(state.get()) == liveStates.end();
              }), this->initialStates.end());
      for (auto &state: this->states) {
        for (auto it = state->next.begin(); it != state->next.end();) {
          auto &[action, transitions] = *it;
          transitions.erase(std::remove_if(transitions.begin(), transitions.end(), [&](const auto &transition) {
            return liveStates.find(transition.target) == liveStates.end();
          }), transitions.end());
          if (it->second.empty()) {
            it = state->next.erase(it);
          } else {
            ++it;
          }
        }
      }
    }

    for (const auto &state: this->states) {
      if (liveTransitions.find(state.get()) == liveTransitions.end()) {
        state->next.clear();
      } else {
        std::unordered_map<Alphabet, boost::unordered_set<learnta::TATransition>> liveNextTransitions;
        for (const auto &[action, transition]: liveTransitions.at(state.get())) {
          if (liveNextTransitions.find(action) == liveNextTransitions.end()) {
            liveNextTransitions[action] = {transition};
          } else {
            liveNextTransitions.at(action).insert(transition);
          }
        }
        for (auto it = state->next.begin(); it != state->next.end();) {
          auto &[action, transitions] = *it;
          if (liveNextTransitions.find(action) == liveNextTransitions.end()) {
            it = state->next.erase(it);
          } else {
            transitions.erase(std::remove_if(transitions.begin(), transitions.end(), [&](const auto &transition) {
              return liveNextTransitions.at(it->first).find(transition) == liveNextTransitions.at(it->first).end();
            }), transitions.end());
            if (transitions.empty()) {
              it = state->next.erase(it);
            } else {
              ++it;
            }
          }
        }

      }
    }

    return *this;
  }

  /*!
   * @brief Check if the outgoing transitions are deterministic
   */
  bool TAState::deterministic() const {
    return std::all_of(this->next.begin(), this->next.end(), [](const auto &pair) -> bool {
      const auto &[action, transitions] = pair;
      for (auto it = transitions.begin(); it != transitions.end(); ++it) {
        if (std::any_of(transitions.begin(), it, [&](const TATransition &transition) -> bool {
          auto constraint = transition.guard;
          constraint.reserve(constraint.size() + it->guard.size());
          constraint.insert(constraint.end(), it->guard.begin(), it->guard.end());

          return satisfiable(constraint);
        })) {
          return false;
        }
      }

      return true;
    });
  }
}
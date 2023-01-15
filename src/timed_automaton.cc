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
          return satisfiable(conjunction(transition.guard, it->guard));
        })) {
          return false;
        }
      }

      return true;
    });
  }

  void TAState::addUpperBoundForUnobservableTransitions() {
    auto it = this->next.find(learnta::UNOBSERVABLE);
    if (it != this->next.end()) {
      for (auto &transition: it->second) {
        addUpperBound(transition.guard);
      }
    }
  }

  void TAState::removeTransitionsWithWeakerGuards() {
    for (auto &[action, transitions]: this->next) {
      // Remove weaker guards
      for (auto it2 = transitions.begin(); it2 != transitions.end();) {
        if (std::any_of(transitions.begin(), transitions.end(), [&](const TATransition &transition) -> bool {
          return *it2 != transition && isWeaker(transition.guard, it2->guard) && transition.target == it2->target;
        })) {
          it2 = transitions.erase(it2);
        } else {
          ++it2;
        }
      }
    }
  }

  void TAState::mergeNondeterministicBranchingWithSameTarget() {
    for (auto &[action, transitions]: this->next) {
      for (auto it2 = transitions.begin(); it2 != transitions.end(); ++it2) {
        for (auto it3 = std::next(it2); it3 != transitions.end();) {
          if (it2->target == it3->target && satisfiable(conjunction(it2->guard, it3->guard))) {
#ifdef DEBUG
            BOOST_LOG_TRIVIAL(debug) << "The conjunction of " << it2->guard << " and "
                                     << it3->guard << " is satisfiable";
#endif
            // Use the reset and target causing more imprecise clocks
            //if (it2->resetVars.size() < it3->resetVars.size()) {
            if (TATransition::impreciseConstantAssignSize(it2->resetVars) <
                TATransition::impreciseConstantAssignSize(it3->resetVars)) {
              it3->addPreciseConstantAssignments(it2->resetVars);
              it2->resetVars = it3->resetVars;
            } else {
              it2->addPreciseConstantAssignments(it3->resetVars);
            }
            std::vector<std::vector<Constraint>> guards = {it2->guard, it3->guard};
            it2->guard = unionHull(guards);
            it3 = transitions.erase(it3);
          } else {
            ++it3;
          }
        }
      }
    }
  }

  bool TAState::needSplitting() const {
    for (auto &[action, transitions]: this->next) {
      for (auto it2 = transitions.begin(); it2 != transitions.end(); ++it2) {
        for (auto it3 = std::next(it2); it3 != transitions.end(); ++it3) {
          if (it2->target != it3->target && satisfiable(conjunction(it2->guard, it3->guard)) &&
              simpleVariables(it2->guard) == simpleVariables(it3->guard)) {
            return true;
          }
        }
      }
    }

    return false;
  }

  void TAState::mergeNondeterministicBranching() {
    for (auto &[action, transitions]: this->next) {
      for (auto it2 = transitions.begin(); it2 != transitions.end(); ++it2) {
        for (auto it3 = std::next(it2); it3 != transitions.end();) {
          if (satisfiable(conjunction(it2->guard, it3->guard))) {
#ifdef DEBUG
            BOOST_LOG_TRIVIAL(debug) << "The conjunction of " << it2->guard << " and "
                                     << it3->guard << " is satisfiable";
#endif
            // assert(it2->target == it3->target);
            // It is fine to merge two transitions if their targets are "equivalent"
            // However, it is not straightforward to check the equivalence.
            // So, we tentatively weaken the requirement
            assert(it2->target->isMatch == it3->target->isMatch);
            if (it2->target != it3->target) {
              BOOST_LOG_TRIVIAL(warning) << "We merge transitions with different targets. This is unstable";
              // Check if this happens only when the imprecise clocks are different
              if (simpleVariables(it2->guard) == simpleVariables(it3->guard)) {
                BOOST_LOG_TRIVIAL(warning) << "Moreover, the merged transitions have the same set of imprecise clocks";
                BOOST_LOG_TRIVIAL(warning) << it2->guard;
                BOOST_LOG_TRIVIAL(warning) << it3->guard;
              }
            }
            // Use the reset and target causing more imprecise clocks
            //if (it2->resetVars.size() < it3->resetVars.size()) {
            if (TATransition::impreciseConstantAssignSize(it2->resetVars) <
                TATransition::impreciseConstantAssignSize(it3->resetVars)) {
              it3->addPreciseConstantAssignments(it2->resetVars);
              it2->resetVars = it3->resetVars;
              it2->target = it3->target;
            } else {
              it2->addPreciseConstantAssignments(it3->resetVars);
            }
            std::vector<std::vector<Constraint>> guards = {it2->guard, it3->guard};
            it2->guard = unionHull(guards);
            it3 = transitions.erase(it3);
          } else {
            ++it3;
          }
        }
      }
    }
  }

  void TAState::mergeNondeterministicBranching(const std::unordered_set<ClockVariables> &preciseClocks) {
    for (auto &[action, transitions]: this->next) {
      for (auto it2 = transitions.begin(); it2 != transitions.end(); ++it2) {
        for (auto it3 = std::next(it2); it3 != transitions.end();) {
          if (satisfiable(conjunction(it2->guard, it3->guard))) {
            if (it2->target != it3->target) {
              const auto preciseVariables2 = simpleVariables(it2->guard);
              const auto preciseVariables3 = simpleVariables(it3->guard);
              bool use3 = false;
              for (const auto &preciseClock: preciseClocks) {
                bool preciseIn2 = std::find(preciseVariables2.begin(), preciseVariables2.end(), preciseClock) !=
                                  preciseVariables2.end();
                bool preciseIn3 = std::find(preciseVariables3.begin(), preciseVariables3.end(), preciseClock) !=
                                  preciseVariables3.end();
                if (preciseIn2 != preciseIn3) {
                  use3 = preciseIn3;
                  break;
                }
              }
              if (use3) {
                it2->guard = it3->guard;
                it2->resetVars = it3->resetVars;
                it2->target = it3->target;
              }
              it3 = transitions.erase(it3);
            } else {
              if (TATransition::impreciseConstantAssignSize(it2->resetVars) <
                  TATransition::impreciseConstantAssignSize(it3->resetVars)) {
                it3->addPreciseConstantAssignments(it2->resetVars);
                it2->resetVars = it3->resetVars;
                it2->target = it3->target;
              } else {
                it2->addPreciseConstantAssignments(it3->resetVars);
              }
              std::vector<std::vector<Constraint>> guards = {it2->guard, it3->guard};
              it2->guard = unionHull(guards);
              it3 = transitions.erase(it3);
            }
          } else {
            ++it3;
          }
        }
      }
    }
  }
}
#pragma once

#include <array>
#include <climits>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <vector>
#include <queue>
#include <optional>
#include <cassert>
#include <boost/log/trivial.hpp>

#include "common_types.hh"
#include "constraint.hh"

namespace learnta {
  struct TATransition;

  /*!
   * @brief A state of timed automata
   */
  struct TAState {
    //! @brief The value is true if and only if the state is an accepting state.
    bool isMatch;
    /*!
      @brief A mapping of a character to the transitions.
      @note Because of non-determinism, the second element is a vector.
     */
    std::unordered_map<Alphabet, std::vector<TATransition>> next;

    explicit TAState(bool isMatch = false) : isMatch(isMatch) { next.clear(); }

    TAState(bool isMatch,
            std::unordered_map<Alphabet, std::vector<TATransition>> next)
            : isMatch(isMatch), next(std::move(next)) {}
    /*!
     * @brief Check if the transitions is deterministic.
     */
    [[nodiscard]] bool deterministic() const;
  };

/*!
  @brief A state of timed automata
 */
  struct TATransition {
    //! @brief The pointer to the target state.
    TAState *target{};
    /*!
     * @brief The clock variables reset after this transition.
     *
     * clock[i] is reset to 0 if resetVars[i].has_value() == false. clock[i] is reset to clock[*resetVars[i]] otherwise
     */
    std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>> resetVars;
    //! @brief The guard for this transition.
    std::vector<Constraint> guard;

    TATransition() = default;

    TATransition(TAState *target,
                 std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>> resetVars,
                 std::vector<Constraint> guard)
            : target(target), resetVars(std::move(resetVars)), guard(std::move(guard)) {}

    TATransition(TAState *target, ClockVariables resetVar, std::vector<Constraint> guard) :
            target(target), guard(std::move(guard)) {
      resetVars.emplace_back(resetVar, std::nullopt);
    }

    TATransition(TATransition const &) = default;

    bool operator==(const TATransition &rhs) const {
      return target == rhs.target &&
             resetVars == rhs.resetVars &&
             guard == rhs.guard;
    }

    bool operator!=(const TATransition &rhs) const {
      return !(rhs == *this);
    }

    ~TATransition() = default;
  };

  inline std::size_t hash_value(const TATransition &transition) {
    return boost::hash_value(std::make_tuple(transition.target, transition.resetVars, transition.guard));
  }

  /*!
   * @brief A timed automaton
   */
  struct TimedAutomaton : public Automaton<TAState> {
    //! @brief The maximum constraints for each clock variables.
    std::vector<int> maxConstraints;

    /*!
      @brief make a deep copy of this timed automaton.

      @param [out] dest The destination of the deep copy.
      @param [out] old2new The mapping from the original state to the
      corresponding new state.
     */
    void deepCopy(
            TimedAutomaton &dest,
            std::unordered_map<TAState *, std::shared_ptr<TAState>> &old2new) const {
      // copy states
      old2new.reserve(stateSize());
      dest.states.reserve(stateSize());
      for (const auto &oldState: states) {
        dest.states.emplace_back(std::make_shared<TAState>(*oldState));
        old2new[oldState.get()] = dest.states.back();
      }
      // copy initial states
      dest.initialStates.reserve(initialStates.size());
      for (const auto &oldInitialState: initialStates) {
        dest.initialStates.emplace_back(old2new[oldInitialState.get()]);
      }
      // modify dest of transitions
      for (auto &state: dest.states) {
        for (auto &edges: state->next) {
          for (auto &edge: edges.second) {
            auto oldTarget = edge.target;
            edge.target = old2new[oldTarget].get();
          }
        }
      }
      dest.maxConstraints = maxConstraints;
    }

    //! @brief Returns the number of clock variables used in the timed automaton.
    [[nodiscard]] inline size_t clockSize() const {
      return maxConstraints.size();
    }

    /*!
     * @brief Take the complement of this timed automaton
     *
     * @pre This timed automaton is deterministic and the transitions are complete
     */
    [[nodiscard]] TimedAutomaton complement(const std::vector<Alphabet> &alphabet) const {
      TimedAutomaton result;
      std::unordered_map<TAState *, std::shared_ptr<TAState>> old2new;
      deepCopy(result, old2new);
      result.makeComplete(alphabet);
      assert(this->stateSize() + 1 == result.stateSize());
      for (auto &state: result.states) {
        state->isMatch = !state->isMatch;
      }

      return result;
    }

    /*!
     * @brief Add transitions to make it complete
     */
    void makeComplete(const std::vector<Alphabet> &alphabet) {
      this->states.push_back(std::make_shared<TAState>(false));
      if (this->initialStates.empty()) {
        this->initialStates.push_back(this->states.back());
      }
      // If the transition is empty, we make a transition to the sink state
      for (auto &state: this->states) {
        for (const auto &action: alphabet) {
          if (state->next.find(action) == state->next.end() || state->next.at(action).empty()) {
            state->next[action].emplace_back();
            state->next.at(action).back().target = this->states.back().get();
          } else {
            std::vector<std::vector<Constraint>> disjunctiveGuard;
            disjunctiveGuard.reserve(state->next.at(action).size());
            for (const auto &transition: state->next.at(action)) {
              if (transition.guard.empty()) {
                disjunctiveGuard.clear();
                break;
              }
              disjunctiveGuard.push_back(transition.guard);
            }
            if (disjunctiveGuard.empty()) {
              continue;
            }
            const auto complement = negate(disjunctiveGuard);
            state->next.at(action).reserve(complement.size());
            for (const auto &constraints: complement) {
              state->next.at(action).emplace_back(this->states.back().get(),
                                                  decltype(std::declval<learnta::TATransition>().resetVars){},
                                                  constraints);
            }
          }
        }
      }
    };

    //! @brief simplify the transitions by reducing the duplications
    void simplifyTransitions() {
      for (auto &state: this->states) {
        std::unordered_map<Alphabet, std::vector<learnta::TATransition>> newNext;
        for (auto &[action, transitions]: state->next) {
          std::vector<learnta::TATransition> reducedTransitions;
          for (const auto &transition: transitions) {
            auto it = std::find_if(reducedTransitions.begin(), reducedTransitions.end(), [&](const auto &another) {
              return transition.target == another.target && transition.resetVars == another.resetVars &&
                     isWeaker(another.guard, transition.guard);
            });
            if (it == reducedTransitions.end()) {
              reducedTransitions.push_back(transition);
            }
          }
          newNext[action] = reducedTransitions;
        }
        state->next = newNext;
      }
    }

    //! @brief Remove the transitions that are unreachable from the initial states abstracting the timing constraints
    void removeTriviallyUnreachableStates() {
      std::unordered_set<TAState *> reachableStates;
      std::deque<TAState *> currentQueue;
      for (const auto &initialState: this->initialStates) {
        reachableStates.insert(initialState.get());
        currentQueue.push_back(initialState.get());
      }

      // Make the set of trivially reachable states
      while (!currentQueue.empty()) {
        const auto currentState = currentQueue.front();
        currentQueue.pop_front();
        for (const auto &[action, transitions]: currentState->next) {
          for (const auto &transition: transitions) {
            if (reachableStates.find(transition.target) == reachableStates.end()) {
              reachableStates.insert(transition.target);
              currentQueue.push_back(transition.target);
            }
          }
        }
      }

      if (reachableStates.size() != this->stateSize()) {
        // remove redundant states
        BOOST_LOG_TRIVIAL(info) << "There are " << this->stateSize() - reachableStates.size() << " redundant states";
        for (auto it = this->states.begin(); it != this->states.end();) {
          if (reachableStates.find(it->get()) == reachableStates.end()) {
            it = this->states.erase(it);
          } else {
            ++it;
          }
        }
        assert(this->stateSize() == reachableStates.size());

        // Update the max constraints
        this->maxConstraints = TimedAutomaton::makeMaxConstants(this->states);
      }
    }

    //! @brief Remove self loop of non-accepting locations
    void removeDeadLoop() {
      std::deque<std::shared_ptr<TAState>> nonAccepting;
      std::copy_if(this->states.begin(), this->states.end(), std::back_inserter(nonAccepting),
                   [](const std::shared_ptr<TAState> &state) {
                     return !state->isMatch;
                   });
      while (!nonAccepting.empty()) {
        std::shared_ptr<TAState> current = nonAccepting.front();
        nonAccepting.pop_front();
        // We do not remove the initial states
        if (std::find(this->initialStates.begin(), this->initialStates.end(), current) != this->initialStates.end()) {
          continue;
        }
        if (std::all_of(current->next.begin(), current->next.end(), [&](const auto &pair) {
          return std::all_of(pair.second.begin(), pair.second.end(), [&](const auto &transition) {
            return transition.target == current.get();
          });
        })) {
          // We remove only the transitions for the totality
          current->next.clear();
        }
      }
    }

    /*!
     * @brief Remove trivially dead states, i.e., unreachable to any of the accepting states as a graph
     */
    void removeTriviallyDeadStates() {
      std::unordered_map<TAState *, std::unordered_set<TAState *>> backwardEdges;
      for (const auto &state: this->states) {
        for (const auto &[action, transitions]: state->next) {
          for (const auto &transition: transitions) {
            auto it = backwardEdges.find(transition.target);
            if (it == backwardEdges.end()) {
              backwardEdges[transition.target] = {state.get()};
            } else {
              backwardEdges.at(transition.target).insert(state.get());
            }
          }
        }
      }

      // The states potentially reachable to an accepting state
      std::unordered_set<TAState *> liveStates;
      std::queue<TAState *> newLiveStates;
      for (const auto &state: this->states) {
        if (state->isMatch) {
          liveStates.insert(state.get());
          newLiveStates.push(state.get());
        }
      }
      while (!newLiveStates.empty()) {
        const auto newLiveState = newLiveStates.front();
        newLiveStates.pop();
        if (backwardEdges.find(newLiveState) != backwardEdges.end()) {
          for (const auto &backwardState: backwardEdges.at(newLiveState)) {
            if (liveStates.find(backwardState) == liveStates.end()) {
              liveStates.insert(backwardState);
              newLiveStates.push(backwardState);
            }
          }
        }
      }

      if (liveStates.size() != this->stateSize()) {
        // Remove trivially dead states if exists
        BOOST_LOG_TRIVIAL(info) << "There are " << this->stateSize() - liveStates.size() << " dead states";
        this->states.erase(std::remove_if(this->states.begin(), this->states.end(), [&](const auto &state) {
          return liveStates.find(state.get()) == liveStates.end();
        }), this->states.end());
        this->initialStates.erase(
                std::remove_if(this->initialStates.begin(), this->initialStates.end(), [&](const auto &state) {
                  return liveStates.find(state.get()) == liveStates.end();
                }), this->initialStates.end());
        for (const auto &state: this->states) {
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
    }

    //! @brief Remove self loop of non-accepting locations
    TimedAutomaton removeUselessTransitions() {
      std::deque<std::shared_ptr<TAState>> nonAccepting;
      std::copy_if(this->states.begin(), this->states.end(), std::back_inserter(nonAccepting),
                   [](const std::shared_ptr<TAState> &state) {
                     return !state->isMatch;
                   });
      while (!nonAccepting.empty()) {
        std::shared_ptr<TAState> current = nonAccepting.front();
        nonAccepting.pop_front();
        // We do not remove the initial states
        if (std::find(this->initialStates.begin(), this->initialStates.end(), current) != this->initialStates.end()) {
          continue;
        }
        if (current->next.empty() || std::all_of(current->next.begin(), current->next.end(), [&](const auto &pair) {
          return std::all_of(pair.second.begin(), pair.second.end(), [&](const auto &transition) {
            return transition.target == current.get();
          });
        })) {
          this->states.erase(std::remove(this->states.begin(), this->states.end(), current), this->states.end());
          // Remove the transition to the removed location
          for (auto &state: this->states) {
            for (auto it = state->next.begin(); it != state->next.end();) {
              auto &[action, transitions] = *it;
              transitions.erase(std::remove_if(transitions.begin(), transitions.end(), [&](const auto &transition) {
                return transition.target == current.get();
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
     * @brief Optimize the timed automaton by removing unused clock variables
     */
    void removeUnusedClockVariables() {
      std::unordered_set<ClockVariables> usedClockVariables;
      // Make the set of clock variables
      for (const auto &state: this->states) {
        for (const auto &[action, transitions]: state->next) {
          for (const auto &transition: transitions) {
            for (const auto &guard: transition.guard) {
              usedClockVariables.insert(guard.x);
            }
            for (const auto &[assignedVar, usedVarOpt]: transition.resetVars) {
              if (usedVarOpt) {
                usedClockVariables.insert(*usedVarOpt);
              }
            }
          }
        }
      }

      // Construct a map showing how we rename clock variables
      std::vector<ClockVariables> usedClockVariablesVec{usedClockVariables.begin(), usedClockVariables.end()};
      std::sort(usedClockVariablesVec.begin(), usedClockVariablesVec.end());
      // Clock variable x is renamed to clockRenaming.at(x)
      std::unordered_map < ClockVariables, ClockVariables > clockRenaming;
      for (int i = 0; i < usedClockVariablesVec.size(); ++i) {
        clockRenaming[usedClockVariablesVec.at(i)] = i;
      }

      // Rename the clock variables
      for (const auto &state: this->states) {
        for (auto &[action, transitions]: state->next) {
          for (auto &transition: transitions) {
            for (auto &guard: transition.guard) {
              guard.x = clockRenaming.at(guard.x);
            }
            for (auto it = transition.resetVars.begin(); it != transition.resetVars.end();) {
              if (usedClockVariables.find(it->first) == usedClockVariables.end()) {
                it = transition.resetVars.erase(it);
              } else {
                it->first = clockRenaming.at(it->first);
                if (it->second) {
                  *it->second = clockRenaming.at(*it->second);
                }
                ++it;
              }
            }
          }
        }
      }

      // Rename the max constraints
      for (int i = 0; i < usedClockVariablesVec.size(); ++i) {
        this->maxConstraints.at(i) = this->maxConstraints.at(usedClockVariablesVec.at(i));
      }
      this->maxConstraints.erase(this->maxConstraints.begin() + usedClockVariablesVec.size(),
                                 this->maxConstraints.end());
    }

    //! @brief Simplify the timed automaton
    TimedAutomaton simplify() {
      this->simplifyTransitions();
      this->removeTriviallyUnreachableStates();
      this->removeUnusedClockVariables();

      return *this;
    }

    /*!
     * @brief Simplify the timed automaton
     *
     * This function may make the resulting TA incomplete.
     */
    TimedAutomaton simplifyStrong() {
      this->simplifyTransitions();
      this->removeTriviallyUnreachableStates();
      this->removeTriviallyDeadStates();
      this->removeDeadLoop();
      this->removeUselessTransitions();
      this->removeUnusedClockVariables();

      return *this;
    }

    /*!
     * @brief Simplify the timed automaton
     *
     * This function uses zone graph to remove useless states and transitions.
     */
    TimedAutomaton simplifyWithZones();

    /*!
     * @brief Make a vector showing maximum constants for each variable from a set of states
     */
    static std::vector<int> makeMaxConstants(const std::vector<std::shared_ptr<TAState>> &states) {
      std::vector<int> maxConstants;
      for (const auto &state: states) {
        for (const auto &[action, transitions]: state->next) {
          for (const auto &transition: transitions) {
            for (const auto &guard: transition.guard) {
              if (guard.x >= maxConstants.size()) {
                maxConstants.resize(guard.x + 1);
              }
              maxConstants[guard.x] = std::max(maxConstants[guard.x], guard.c);
            }
            for (const auto &[resetVar, updatedVarOpt]: transition.resetVars) {
              if (resetVar >= maxConstants.size()) {
                maxConstants.resize(resetVar + 1);
              }
              if (updatedVarOpt && *updatedVarOpt >= maxConstants.size()) {
                maxConstants.resize(*updatedVarOpt + 1);
              }
            }
          }
        }
      }

      return maxConstants;
    }

    [[nodiscard]] bool deterministic() const {
      return std::all_of(this->states.begin(), this->states.end(), std::mem_fn(&TAState::deterministic));
    }
  };

  static inline std::ostream &operator<<(std::ostream &os,
                                         const TimedAutomaton &TA) {
    std::unordered_map<TAState *, bool> isInit;
    std::unordered_map<TAState *, unsigned int> stateNumber;

    for (unsigned int i = 0; i < TA.states.size(); ++i) {
      // Check if the state is initial for each state
      isInit[TA.states.at(i).get()] =
              std::find(TA.initialStates.begin(), TA.initialStates.end(),
                        TA.states.at(i)) != TA.initialStates.end();
      // Assign a number for each state
      stateNumber[TA.states.at(i).get()] = i;
    }
    os << "digraph G {\n";

    for (const std::shared_ptr<TAState> &state: TA.states) {
      os << "        loc" << stateNumber.at(state.get())
         << " [init=" << isInit.at(state.get()) << ", match=" << state->isMatch
         << "]\n";
    }

    for (const std::shared_ptr<TAState> &source: TA.states) {
      for (const auto &edges: source->next) {
        for (const TATransition &edge: edges.second) {
          TAState *target = edge.target;
          os << "        loc" << stateNumber.at(source.get()) << "->loc"
             << stateNumber.at(target) << " [label=\"" << edges.first << "\"";
          if (!edge.guard.empty()) {
            os << ", guard=\"{";
            bool isFirst = true;
            for (const Constraint guard: edge.guard) {
              if (!isFirst) {
                os << ", ";
              }
              os << guard;
              isFirst = false;
            }
            os << "}\"";
          }
          if (!edge.resetVars.empty()) {
            os << ", reset=\"{";
            bool isFirst = true;
            for (const auto &[resetVar, newVar]: edge.resetVars) {
              if (!isFirst) {
                os << ", ";
              }
              os << "x" << int(resetVar) << " := ";
              if (newVar) {
                os << "x" << int(newVar.value());
              } else {
                os << "0";
              }
              isFirst = false;
            }
            os << "}\"";
          }
          os << "]\n";
        }
      }
    }
    os << "}\n";
    return os;
  }
}

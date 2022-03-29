#pragma once

#include <array>
#include <climits>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <utility>
#include <valarray>
#include <vector>
#include <queue>

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
  };

/*!
  @brief A state of timed automata
 */
  struct TATransition {
    //! @brief The pointer to the target state.
    TAState *target;
    /*!
     * @brief The clock variables reset after this transition.
     *
     * clock[i] is reset to 0 if resetVars[i].has_value() == false. clock[i] is reset to clock[*resetVars[i]] otherwise
     */
    std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>> resetVars;
    //! @brief The guard for this transition.
    std::vector<Constraint> guard;

    TATransition() {}

    TATransition(TAState *target,
                 std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>> resetVars,
                 std::vector<Constraint> guard)
            : target(target), resetVars(std::move(resetVars)), guard(std::move(guard)) {}
  };

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
      // If the transition is empty, we make a transition to the sink state
      for (auto &state: this->states) {
        for (const auto &action: alphabet) {
          if (state->next.find(action) == state->next.end()) {
            state->next[action].emplace_back();
            state->next.at(action).back().target = this->states.back().get();
          }
        }
      }
    };

    //! @brief simplify the transitions by reducing the duplications
    void simplifyTransitions() {
      for (auto &state: this->states) {
        std::unordered_map<Alphabet, std::vector<learnta::TATransition>> newNext;
        for (auto&[action, transitions]: state->next) {
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

    //! @brief Remove self loop of non-accepting locations
    void removeDeadLoop() {
      std::deque<std::shared_ptr<TAState>> nonAccepting;
      std::copy_if(this->states.begin(), this->states.end(), std::back_inserter(nonAccepting),
                   [](const std::shared_ptr<TAState>& state) {
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
          return std::all_of(pair.second.begin(), pair.second.end(), [&] (const auto &transition) {
            return transition.target == current.get();
          });
        })) {
          this->states.erase(std::remove(this->states.begin(), this->states.end(), current), this->states.end());
          // Remove the transition to the removed location
          for (auto &state: this->states) {
            for (auto it = state->next.begin(); it != state->next.end(); ) {
             auto &[action, transitions] = *it;
              transitions.erase(std::remove_if(transitions.begin(), transitions.end(), [&] (const auto &transition){
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
    }

    //! @brief Simplify the timed automaton
    TimedAutomaton simplify() {
      this->simplifyTransitions();
      this->removeDeadLoop();

      return *this;
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
      stateNumber[TA.states.at(i).get()] = i + 1;
    }
    os << "digraph G {\n";

    for (const std::shared_ptr<TAState> &state: TA.states) {
      os << "        loc" << stateNumber.at(state.get())
         << " [init=" << isInit.at(state.get()) << ", match=" << state->isMatch
         << "]\n";
    }

    for (const std::shared_ptr<TAState> &source: TA.states) {
      for (auto edges: source->next) {
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
            for (const auto&[resetVar, newVar]: edge.resetVars) {
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

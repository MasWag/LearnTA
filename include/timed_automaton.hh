#pragma once

#include <array>
#include <climits>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <utility>
#include <valarray>
#include <vector>

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
    using X = ConstraintMaker;
    using TATransition = TATransition;
    using State = learnta::TAState;

    //! @brief The maximum constraints for each clock variables.
    std::vector<int> maxConstraints;

    /*!
      @brief make a deep copy of this timed automaton.

      @param [out] dest The destination of the deep copy.
      @param [out] old2new The mapping from the original state to the
      corresponding new state.
     */
    void deepCopy (
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
    [[nodiscard]] TimedAutomaton complement() const {
      TimedAutomaton result;
      std::unordered_map<TAState *, std::shared_ptr<TAState>> old2new;
      deepCopy(result, old2new);
      for (auto& state: result.states) {
        state->isMatch = !state->isMatch;
      }

      return result;
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

    for (const std::shared_ptr<TAState>& source: TA.states) {
      for (auto edges: source->next) {
        for (const TATransition& edge: edges.second) {
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
            for (const auto& [resetVar, newVar]: edge.resetVars) {
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

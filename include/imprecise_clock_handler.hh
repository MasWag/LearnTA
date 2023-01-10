/**
 * @author Masaki Waga
 * @date 2023/01/10.
 */

#pragma once

#include <stack>

#include "timed_automaton.hh"
#include "renaming_relation.hh"
#include "forward_regional_elementary_language.hh"
#include "neighbor_conditions.hh"

namespace learnta {
  /*!
   * @brief Relax guards to handle imprecise clock variables
   */
  class ImpreciseClockHandler {
  private:
    std::stack<std::pair<TAState *, NeighborConditions>> impreciseNeighbors;

    [[nodiscard]] static std::optional<std::pair<TAState *, NeighborConditions>>
    handleOne(const NeighborConditions &neighbor,
              const TATransition &transition, const NeighborConditions &neighborSuccessor,
              std::vector<TATransition> &newTransitions, bool &matchBounded, bool &noMatch) {
      // Relax the guard if it matches
      if (neighbor.match(transition)) {
        noMatch = false;
#ifdef DEBUG
        BOOST_LOG_TRIVIAL(debug) << "matched! " << "guard: " << transition.guard;
#endif
        const bool upperBounded = std::any_of(transition.guard.begin(), transition.guard.end(),
                                              std::mem_fn(&Constraint::isUpperBound));
        matchBounded = matchBounded || upperBounded;
        BOOST_LOG_TRIVIAL(debug) << "matchBounded: " << matchBounded;
        auto relaxedGuard = neighbor.toRelaxedGuard();
        if (!upperBounded) {
          // Remove upper bound if the matched guard has no upper bound
          relaxedGuard.erase(std::remove_if(relaxedGuard.begin(), relaxedGuard.end(),
                                            [](const auto &constraint) {
                                              return constraint.isUpperBound();
                                            }), relaxedGuard.end());
        }
        BOOST_LOG_TRIVIAL(debug) << "relaxed guard: " << relaxedGuard;
        if (isWeaker(relaxedGuard, transition.guard) && !isWeaker(transition.guard, relaxedGuard)) {
#ifdef DEBUG
          BOOST_LOG_TRIVIAL(debug) << "Relaxed!!";
#endif
          newTransitions.emplace_back(transition.target, transition.resetVars, std::move(relaxedGuard));
          // Follow the transition if it is internal
          if (transition.resetVars.size() == 1 &&
              transition.resetVars.front().first == neighbor.getClockSize() &&
              transition.resetVars.front().second.index() == 0 &&
              std::get<double>(transition.resetVars.front().second) == 0.0) {
            return std::make_pair(transition.target, neighborSuccessor);
          } else {
            const auto impreciseClocks = neighborSuccessor.impreciseClocks();
            if (!std::all_of(impreciseClocks.begin(), impreciseClocks.end(), [&] (const auto &clock) {
              // Check if the imprecise clock is updated
              auto it = std::find_if(transition.resetVars.begin(), transition.resetVars.end(),
                                     [&] (const auto &pair) {
                                       return pair.first == clock;
                                     });
              // Check if the imprecise clock is used
              auto it2 = std::find_if(transition.resetVars.begin(), transition.resetVars.end(),
                                      [&] (const auto &pair) {
                                        return pair.second.index() == 1 && std::get<ClockVariables>(pair.second) == clock;
                                      });
              return it != transition.resetVars.end() && it2 == transition.resetVars.end();
            })) {
              // There are imprecise clock variables after external transition
              // Construct the neighbor successor after external transition
              if (transition.resetVars.back().first == neighbor.getClockSize() &&
                  transition.resetVars.back().second.index() == 0 &&
                  std::get<double>(transition.resetVars.back().second) == 0.0) {
                return std::make_pair(transition.target, neighborSuccessor.applyResets(transition.resetVars));
              } else {
                // Such a case is not supported
                abort();
                return std::make_pair(transition.target, neighborSuccessor);
              }
            }
          }
        }
      }

      return std::nullopt;
    }
  public:
    /*!
     * @brief Add new transition with imprecise clocks
     */
    void push(TAState *jumpedState, const RenamingRelation &renamingRelation,
              const ForwardRegionalElementaryLanguage &sourceElementary,
              const ForwardRegionalElementaryLanguage &targetElementary) {
      // There are imprecise clocks
      if (renamingRelation.impreciseClocks(sourceElementary.getTimedCondition(),
                                           targetElementary.getTimedCondition())) {
        BOOST_LOG_TRIVIAL(debug) << "new imprecise neighbors set is added: " << jumpedState << ", "
                                 << targetElementary << ", " << renamingRelation;
        impreciseNeighbors.emplace(jumpedState,
                                   NeighborConditions{targetElementary, renamingRelation.rightVariables()});
      }
    }

    /*!
     * @brief Relax the guards if necessary
     */
    void run() {
      while (!impreciseNeighbors.empty()) {
        auto [state, neighbor] = impreciseNeighbors.top();
        impreciseNeighbors.pop();
        bool matchBounded;
        bool noMatch = true;
        do {
#ifdef DEBUG
          BOOST_LOG_TRIVIAL(debug) << "current imprecise neighbors: " << state << ", " << neighbor;
#endif
          matchBounded = false;
          // Loop over successors
          for (auto &[action, transitions]: state->next) {
            const auto neighborSuccessor = neighbor.successor(action);
            std::vector<TATransition> newTransitions;
            for (const auto &transition: transitions) {
              const auto result = handleOne(neighbor, transition, neighborSuccessor, newTransitions, matchBounded, noMatch);
              if (result) {
                this->impreciseNeighbors.push(*result);
              }
            }
            std::move(newTransitions.begin(), newTransitions.end(), std::back_inserter(transitions));
          }
          neighbor.successorAssign();
        } while (matchBounded || noMatch);
      }
    }
  };
}
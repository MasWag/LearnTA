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
      // TODO: It seems we need to propagate imprecise clocks to exterior transitions, in general
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
            for (auto &transition: transitions) {
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
                    impreciseNeighbors.emplace(transition.target, neighborSuccessor);
                  }
                }
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
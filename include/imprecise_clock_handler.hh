/**
 * @author Masaki Waga
 * @date 2023/01/10.
 */

#pragma once

#include <stack>
#include <boost/unordered_set.hpp>

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
    boost::unordered_set<std::pair<TAState *, NeighborConditions>> impreciseNeighbors;

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
          // Propagate imprecise clocks also to external transitions
          std::size_t targetClockSize = 0;
          for (const auto &[action, transitions]: transition.target->next) {
            for (const auto &cTransition: transitions) {
              std::for_each(cTransition.guard.begin(), cTransition.guard.end(), [&] (const Constraint &constraint) {
                targetClockSize = std::max(targetClockSize, static_cast<std::size_t>(constraint.x + 1));
              });
            }
          }
          // Check if there are imprecise clocks after transition
          if (transition.resetVars.size() >= targetClockSize &&
              std::all_of(transition.resetVars.begin(), transition.resetVars.end(), [&] (const auto &pair) {
                return pair.second.index() == 0;
              })) {
            // If constant values are assigned to all the clock variables
            return std::nullopt;
          }
          const auto impreciseClocks = neighbor.impreciseClocks();
          if (std::all_of(impreciseClocks.begin(), impreciseClocks.end(), [&] (const auto &clock) {
            // Check if the imprecise clock is updated to a precise value
            auto it = std::find_if(transition.resetVars.begin(), transition.resetVars.end(),
                                   [&] (const auto &pair) {
              return pair.first == clock &&
                     (pair.second.index() == 1 ||
                      std::get<double>(pair.second) == std::floor(std::get<double>(pair.second)));
            });
            // Check if the imprecise clock is used
            auto it2 = std::find_if(transition.resetVars.begin(), transition.resetVars.end(),
                                    [&] (const auto &pair) {
              return pair.second.index() == 1 && std::get<ClockVariables>(pair.second) == clock;
            });
            return it != transition.resetVars.end() && it2 == transition.resetVars.end();
          })) {
            // If all the imprecise clocks are overwritten to a precise value
            return std::nullopt;
          }
          // There are imprecise clock variables after external transition
          // Construct the neighbor successor after external transition
          const auto newNeighbor = neighbor.makeAfterExternalTransition(transition.resetVars, targetClockSize);
          BOOST_LOG_TRIVIAL(info) << "New neighbor after external transition: " << transition.target<< ": "
                                   << newNeighbor;
          return std::make_pair(transition.target, newNeighbor);
/*
[2023-01-10 21:25:54.110183] [0x0000000113d62600] [error]   Unimplemented case. target clock size: 2, neighbor: (aa, 6 < T_{0, 0}  < 7 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 1 < T_{1, 1}  < 2 && 7 < T_{1, 2}  < 8 && 6 < T_{2, 2}  < 7, 0 < {x0, x2, }{x1, }) {x2, x0} {
(aa, 6 < T_{0, 0}  < 7 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 1 < T_{1, 1}  < 2 && 7 < T_{1, 2}  < 8 && 6 < T_{2, 2}  < 7, 0 < {x0, x2, }{x1, })
(aa, 5 < T_{0, 0}  < 6 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 2 < T_{1, 1}  < 3 && 9 <= T_{1, 2}  <= 9 && 6 < T_{2, 2}  < 7, 0 <= {x1, }{x0, x2, })
(aa, 6 < T_{0, 0}  < 7 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 1 < T_{1, 1}  < 2 && 8 <= T_{1, 2}  <= 8 && 6 < T_{2, 2}  < 7, 0 <= {x1, }{x0, x2, })
(aa, 6 <= T_{0, 0}  <= 6 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 2 <= T_{1, 1}  <= 2 && 8 < T_{1, 2}  < 9 && 6 < T_{2, 2}  < 7, 0 < {x0, x1, x2, })
(aa, 5 < T_{0, 0}  < 6 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 2 < T_{1, 1}  < 3 && 9 < T_{1, 2}  < 10 && 6 < T_{2, 2}  < 7, 0 < {x1, }{x0, x2, })
(aa, 5 < T_{0, 0}  < 6 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 2 < T_{1, 1}  < 3 && 8 < T_{1, 2}  < 9 && 6 < T_{2, 2}  < 7, 0 < {x0, x2, }{x1, })
(aa, 6 < T_{0, 0}  < 7 && 8 <= T_{0, 1}  <= 8 && 14 < T_{0, 2}  < 15 && 1 < T_{1, 1}  < 2 && 8 < T_{1, 2}  < 9 && 6 < T_{2, 2}  < 7, 0 < {x1, }{x0, x2, })
}, Reset: x0 := x2, x1 := 0.25
 */
          BOOST_LOG_TRIVIAL(error) << "Unimplemented case. "
                                   << "target clock size: " << targetClockSize << ", "
                                   << "neighbor: " << neighbor << ", "
                                   << "Reset: " << transition.resetVars;
          abort();
#if 0
          // There are imprecise clock variables after external transition
          // Construct the neighbor successor after external transition
          if (transition.resetVars.back().first == neighbor.getClockSize() &&
              transition.resetVars.back().second.index() == 0 &&
              std::get<double>(transition.resetVars.back().second) == 0.0) {
            return std::make_pair(transition.target, neighborSuccessor.applyResets(transition.resetVars));
          } else {
            // Such a case is not supported
            BOOST_LOG_TRIVIAL(error) << "Unimplemented case. "
                                     << "neighbor clock size: " << neighbor.getClockSize() << ", "
                                     << "Reset: " << transition.resetVars;
            // abort();
            // return std::make_pair(transition.target, neighborSuccessor);
            }
#endif
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
      std::unordered_set<std::size_t> visitedImpreciseNeighborsHash;
      while (!impreciseNeighbors.empty()) {
        auto [state, neighbor] = *impreciseNeighbors.begin();
        const auto hash = boost::hash_value(*impreciseNeighbors.begin());
        impreciseNeighbors.erase(impreciseNeighbors.begin());
        // Skip if this pair is already visited
        if (visitedImpreciseNeighborsHash.find(hash) != visitedImpreciseNeighborsHash.end()) {
          continue;
        }
        visitedImpreciseNeighborsHash.insert(hash);
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
                this->impreciseNeighbors.insert(*result);
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
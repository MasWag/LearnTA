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
#include "timed_automaton_runner.hh"

namespace learnta {
  /*!
   * @brief Relax guards to handle imprecise clock variables
   */
  class ImpreciseClockHandler {
  private:
    boost::unordered_set<std::pair<TAState *, NeighborConditions>> impreciseNeighbors;

    [[nodiscard]] static std::optional<std::pair<TAState *, NeighborConditions>>
    handleOne(const NeighborConditions &neighbor, const Alphabet action,
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
          const auto preciseClocksAfterReset = neighbor.preciseClocksAfterReset(transition);
          const auto neighborAfterTransition = neighbor.makeAfterTransition(action, transition);
          const auto originalValuation = neighborAfterTransition.toOriginalValuation();
          newTransitions.emplace_back(transition.target,
                                      embedIfImprecise(transition.resetVars,
                                                       preciseClocksAfterReset,
                                                       originalValuation),
                                      std::move(relaxedGuard));
          return std::make_pair(transition.target, neighborAfterTransition);
#if 0
          // Handle the internal transitions
          if (transition.resetVars.size() == 1 &&
              transition.resetVars.front().first == neighbor.getClockSize() &&
              transition.resetVars.front().second.index() == 0 &&
              std::get<double>(transition.resetVars.front().second) == 0.0) {
            // Embed imprecise clocks to the precise range to reduce the error
            const auto originalValuation = TimedAutomatonRunner::applyReset(neighborSuccessor.toOriginalValuation(),
                                                                            transition.resetVars);
            auto resetVars = transition.resetVars;
            // Remove imprecise clocks
            resetVars.erase(std::remove_if(resetVars.begin(), resetVars.end(), [&](const auto &reset) {
              return preciseClocksAfterReset.find(reset.first) == preciseClocksAfterReset.end();
            }), resetVars.end());
            // Add valuations if imprecise
            for (ClockVariables clock = 0; clock < static_cast<ClockVariables>(originalValuation.size()); ++clock) {
              if (preciseClocksAfterReset.find(clock) == preciseClocksAfterReset.end()) {
                resetVars.emplace_back(clock, originalValuation.at(clock));
              }
            }
            newTransitions.emplace_back(transition.target, resetVars, std::move(relaxedGuard));
            // We reconstruct the successor with precise clocks
            return std::make_pair(transition.target, neighborSuccessor.reconstruct(preciseClocksAfterReset));
          } else {
            // Handle the external transitions
            std::size_t targetClockSize = 0;
            for (const auto &[action, transitions]: transition.target->next) {
              for (const auto &cTransition: transitions) {
                std::for_each(cTransition.guard.begin(), cTransition.guard.end(), [&](const Constraint &constraint) {
                  targetClockSize = std::max(targetClockSize, static_cast<std::size_t>(constraint.x + 1));
                });
              }
            }
            // Check if there are imprecise clocks after transition
            if (transition.resetVars.size() >= targetClockSize &&
                std::all_of(transition.resetVars.begin(), transition.resetVars.end(), [&](const auto &pair) {
                  return pair.second.index() == 0;
                })) {
              // If constant values are assigned to all the clock variables
              newTransitions.emplace_back(transition.target, transition.resetVars, std::move(relaxedGuard));
              return std::nullopt;
            }
            const auto impreciseClocks = neighbor.impreciseClocks();
            if (std::all_of(impreciseClocks.begin(), impreciseClocks.end(), [&](const auto &clock) {
              // Check if the imprecise clock is updated to a precise value
              auto it = std::find_if(transition.resetVars.begin(), transition.resetVars.end(),
                                     [&](const auto &pair) {
                                       return pair.first == clock &&
                                              (pair.second.index() == 1 ||
                                               std::get<double>(pair.second) ==
                                               std::floor(std::get<double>(pair.second)));
                                     });
              // Check if the imprecise clock is used
              auto it2 = std::find_if(transition.resetVars.begin(), transition.resetVars.end(),
                                      [&](const auto &pair) {
                                        return pair.second.index() == 1 &&
                                               std::get<ClockVariables>(pair.second) == clock;
                                      });
              return it != transition.resetVars.end() && it2 == transition.resetVars.end();
            })) {
              // If all the imprecise clocks are overwritten to a precise value
              newTransitions.emplace_back(transition.target, transition.resetVars, std::move(relaxedGuard));
              return std::nullopt;
            }
            // There are imprecise clock variables after external transition
            // Construct the neighbor successor after external transition
            // TODO: This part is wrong
            const auto newNeighbor = neighbor.makeAfterExternalTransition(transition.resetVars, targetClockSize);
            const auto originalValuation = TimedAutomatonRunner::applyReset(
                    neighbor.toOriginalValuation(targetClockSize), transition.resetVars);
            auto resetVars = transition.resetVars;
            // Remove imprecise clocks
            resetVars.erase(std::remove_if(resetVars.begin(), resetVars.end(), [&](const auto &reset) {
              return preciseClocksAfterReset.find(reset.first) == preciseClocksAfterReset.end();
            }), resetVars.end());
            // Add valuations if imprecise
            for (ClockVariables clock = 0; clock < static_cast<ClockVariables>(originalValuation.size()); ++clock) {
              if (preciseClocksAfterReset.find(clock) == preciseClocksAfterReset.end()) {
                resetVars.emplace_back(clock, originalValuation.at(clock));
              }
            }
            newTransitions.emplace_back(transition.target, resetVars, std::move(relaxedGuard));
            // We reconstruct the successor with precise clocks
            return std::make_pair(transition.target, newNeighbor.reconstruct(preciseClocksAfterReset));
          }
#endif
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
              const auto result = handleOne(neighbor, action, transition, neighborSuccessor,
                                            newTransitions, matchBounded, noMatch);
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


    static TATransition::Resets embedIfImprecise(TATransition::Resets resets,
                                                 const std::unordered_set<ClockVariables> &preciseClocks,
                                                 const std::vector<double> &embeddedValuation) {
      // Remove imprecise clocks
      resets.erase(std::remove_if(resets.begin(), resets.end(), [&](const auto &reset) {
        return preciseClocks.find(reset.first) == preciseClocks.end();
      }), resets.end());
      // Add valuations if imprecise
      for (ClockVariables clock = 0; clock < static_cast<ClockVariables>(embeddedValuation.size()); ++clock) {
        if (preciseClocks.find(clock) == preciseClocks.end()) {
          resets.emplace_back(clock, embeddedValuation.at(clock));
        }
      }
      return resets;
    }
  };
}
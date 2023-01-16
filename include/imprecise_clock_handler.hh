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
    handleOne(const NeighborConditions &neighbor, const Alphabet action, const TATransition &transition,
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
          if (preciseClocksAfterReset.empty() || neighborAfterTransition.precise()) {
            return std::nullopt;
          } else {
            return std::make_pair(transition.target, neighborAfterTransition);
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
              const ForwardRegionalElementaryLanguage &targetElementary) {
      // There are imprecise clocks
      if (renamingRelation.hasImpreciseClocks(targetElementary.getTimedCondition())) {
        BOOST_LOG_TRIVIAL(debug) << "new imprecise neighbors set is added: " << jumpedState << ", "
                                 << targetElementary << ", " << renamingRelation;
        impreciseNeighbors.emplace(jumpedState, NeighborConditions{targetElementary, renamingRelation.rightVariables()});
      }
    }

    /*!
     * @brief Relax the guards if necessary
     */
    void run() {
      std::unordered_set<std::size_t> visitedImpreciseNeighborsHash;
      while (!impreciseNeighbors.empty()) {
        BOOST_LOG_TRIVIAL(debug) << "visitedImpreciseNeighborsHash size: " << visitedImpreciseNeighborsHash.size();
        BOOST_LOG_TRIVIAL(debug) << "impreciseNeighbors size: " << this->impreciseNeighbors.size();
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
              const auto result = handleOne(neighbor, action, transition,
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
      BOOST_LOG_TRIVIAL(debug) << "ImpreciseClockHandler: finished!";
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
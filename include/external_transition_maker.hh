/**
 * @author Masaki Waga
 * @date 2022/03/27.
 */

#pragma once

#include <utility>
#include <memory>

#include <boost/unordered_map.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

#include "timed_automaton.hh"
#include "renaming_relation.hh"
#include "timed_condition_set.hh"

namespace learnta {
  /*!
   * @brief A class to make a transition from P to ext(P)
   *
   */
  class ExternalTransitionMaker {
  private:
    // (TargetTAState, RenamingRelation) -> SourceTimedCondition
    boost::unordered_map<std::pair<std::shared_ptr<TAState>, RenamingRelation>, TimedConditionSet> sourceMap;
    // (TargetTAState, RenamingRelation) -> TargetTimedCondition
    boost::unordered_map<std::pair<std::shared_ptr<TAState>, RenamingRelation>, TimedConditionSet> targetMap;
  public:
    /*!
     * @brief Add a transition to targetState
     *
     * The transition corresponds to a morphism \f$(u, \Lambda, u', \Lambda', R)\f$, where
     *     - \f$\Lambda\f$ is sourceCondition,
     *     - \f$\Lambda\f$ is targetCondition, and
     *     - \f$R\f$ is renamingRelation,
     */
    void add(const std::shared_ptr<TAState> &targetState, const RenamingRelation &renamingRelation,
             const TimedCondition &sourceCondition, const TimedCondition &targetCondition,
             const std::optional<TimedCondition> &prefixSourceCondition = std::nullopt) {
      auto it = sourceMap.find(std::make_pair(targetState, renamingRelation));
      if (it == sourceMap.end()) {
        sourceMap[std::make_pair(targetState, renamingRelation)] = TimedConditionSet{sourceCondition};
        targetMap[std::make_pair(targetState, renamingRelation)] = TimedConditionSet{targetCondition};
      } else {
        if (prefixSourceCondition && it->second.back().includes(*prefixSourceCondition)) {
          it->second.back().convexHullAssign(sourceCondition);
          targetMap.at(std::make_pair(targetState, renamingRelation)).back().convexHullAssign(targetCondition);
        } else {
          it->second.push_back(sourceCondition);
          targetMap.at(std::make_pair(targetState, renamingRelation)).push_back(targetCondition);
        }
      }
    }

    /*!
     * @brief Include the transition of the immediate exterior of the current conditions
     */
    void includeImmediateExterior(const std::shared_ptr<TAState> &targetState,
                                  const RenamingRelation &renamingRelation) {
      sourceMap[std::make_pair(targetState, renamingRelation)].removeEqualityUpperBoundAssign();
    }

    /*!
     * @brief Generate transitions
     *
     * @todo Check if this is as expected
     */
    [[nodiscard]] std::vector<TATransition> make() const {
      std::vector<TATransition> result;
      result.reserve(sourceMap.size());

      for (const auto&[targetWithRenaming, sourceConditions]: sourceMap) {
        const auto &[target, currentRenamingRelation] = targetWithRenaming;
        const auto targetConditions = targetMap.at(targetWithRenaming);
        assert(sourceConditions.size() == targetConditions.size());
        for (int i = 0; i < sourceConditions.size(); ++i) {
          const auto sourceCondition = sourceConditions.getConditions().at(i);
          const auto targetCondition = targetConditions.getConditions().at(i);
          BOOST_LOG_TRIVIAL(trace) << "Constructing a transition with " << sourceCondition << " and "
                                   << currentRenamingRelation.size();
          auto resets = currentRenamingRelation.toReset(sourceCondition, targetCondition);
          // Initialize the new clock variables
          for (auto resetVariable = sourceCondition.size(); resetVariable < targetCondition.size(); ++resetVariable) {
            resets.emplace_back(resetVariable, 0.0);
          }
          result.emplace_back(target.get(), resets, sourceCondition.toGuard());
        }
      }

      return result;
    }
  };
}

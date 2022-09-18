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
    // Pairs of \f$\Lambda\f$ and \f$ext^t(\Lambda)\f$ such that \f$\Lambda\f$ is a boundary of P and successor(P)
    std::vector<std::pair<TimedCondition, TimedCondition>> boundaryExteriors;

    /*!
     * @brief Construct a reset to a value in the timed condition
     *
     * @pre condition is simple
     */
    static std::vector<double> toValuation(TimedCondition condition) {
      std::vector<double> result;
      result.resize(condition.size());
      for (int i = 0; i < condition.size(); ++i) {
        const auto lowerBound = condition.getLowerBound(i, condition.size() - 1);
        const auto upperBound = condition.getUpperBound(i, condition.size() - 1);
        if (lowerBound.first == -upperBound.first && lowerBound.second && upperBound.second) {
          // When the bound is a point
          result.at(i) = upperBound.first;
        } else {
          // When the bound is not a point
          auto middlePoint = (upperBound.first - lowerBound.first) / 2.0;
          result.at(i) = middlePoint;
          condition.restrictLowerBound(i, condition.size() - 1, Bounds{-middlePoint, true});
          condition.restrictUpperBound(i, condition.size() - 1, Bounds{middlePoint, true});
        }
      }

      return result;
    }
  public:
    /*!
     * @brief Add a transition to targetState
     *
     * The transition corresponds to a morphism \f$(u, \Lambda, u', \Lambda', R)\f$, where
     *     - \f$\Lambda\f$ is sourceCondition,
     *     - \f$\Lambda\f$ is targetCondition, and
     *     - \f$R\f$ is renamingRelation.
     *
     * We can also include the transition from the continuous immediate exterior \f$ext^t(\Lambda)\f$, where
     *  - \f$\Lambda \lor ext^t(\Lambda)\f$ is sourceExternalCondition
     */
    void add(const std::shared_ptr<TAState> &targetState, const RenamingRelation &renamingRelation,
             const TimedCondition &sourceCondition, const TimedCondition &targetCondition,
             const std::optional<TimedCondition> &sourceExternalConditionOpt = std::nullopt) {
      if (sourceExternalConditionOpt) {
        boundaryExteriors.emplace_back(sourceCondition, *sourceExternalConditionOpt);
      }
      auto it = sourceMap.find(std::make_pair(targetState, renamingRelation));
      if (it == sourceMap.end()) {
        sourceMap[std::make_pair(targetState, renamingRelation)] = TimedConditionSet{sourceCondition};
        targetMap[std::make_pair(targetState, renamingRelation)] = TimedConditionSet{targetCondition};
      } else {
        // TODO: Implement constraint merging to generate a smaller DTA
        it->second.push_back(sourceCondition);
        targetMap.at(std::make_pair(targetState, renamingRelation)).push_back(targetCondition);
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
     */
    [[nodiscard]] std::vector<TATransition> make() const {
      std::vector<TATransition> result;
      result.reserve(sourceMap.size());

      for (const auto &[targetWithRenaming, sourceConditions]: sourceMap) {
        const auto &[target, currentRenamingRelation] = targetWithRenaming;
        const auto targetConditions = targetMap.at(targetWithRenaming);
        assert(sourceConditions.size() == targetConditions.size());
        for (int i = 0; i < sourceConditions.size(); ++i) {
          const auto sourceCondition = sourceConditions.getConditions().at(i);
          const auto targetCondition = targetConditions.getConditions().at(i);
          BOOST_LOG_TRIVIAL(trace) << "Constructing a transition with " << sourceCondition << " and "
                                   << currentRenamingRelation.size();
          const auto it = std::find_if(boundaryExteriors.begin(), boundaryExteriors.end(), [&](const auto pair) {
            return sourceCondition.includes(pair.first);
          });
          if (it == boundaryExteriors.end()) {
            // We only have to refresh the new variable
            auto resets = currentRenamingRelation.toReset(sourceCondition, targetCondition);
            // Initialize the new clock variables
            for (auto resetVariable = sourceCondition.size(); resetVariable < targetCondition.size(); ++resetVariable) {
              if (resets.end() != std::find_if(resets.begin(), resets.end(), [&] (const auto &pair) {
                return pair.first == resetVariable;
              })) {
                resets.emplace_back(resetVariable, 0.0);
              }
            }
            result.emplace_back(target.get(), resets, sourceCondition.toGuard());
          } else {
            assert(sourceCondition == it->first);
            // We project to the non-exterior area
            const auto nonExteriorValuation = toValuation(sourceCondition);
            // Map the valuation using the renaming relation
            const auto renamedValuation = currentRenamingRelation.apply<double>(nonExteriorValuation);
            TATransition::Resets resets;
            for (int var = 0; var < renamedValuation.size(); ++var) {
              resets.emplace_back(var, renamedValuation.at(var));
            }
            // Initialize the new clock variables
            for (auto resetVariable = renamedValuation.size(); resetVariable < targetCondition.size(); ++resetVariable) {
              resets.emplace_back(resetVariable, 0.0);
            }
            result.emplace_back(target.get(), resets, it->second.toGuard());
          }
        }
      }

      return result;
    }
  };
}

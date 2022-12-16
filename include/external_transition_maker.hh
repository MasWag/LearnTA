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
   * @todo We need a refactoring once it works
   */
  class ExternalTransitionMaker {
  private:
    // (TargetTAState, RenamingRelation) -> SourceTimedCondition
    boost::unordered_map<std::pair<std::shared_ptr<TAState>, RenamingRelation>, TimedConditionSet> sourceMap;
    // (TargetTAState, RenamingRelation) -> TargetTimedCondition
    boost::unordered_map<std::pair<std::shared_ptr<TAState>, RenamingRelation>, TimedConditionSet> targetMap;
  public:
    /*!
     * @brief Construct a reset to a value in the timed condition
     *
     * @pre condition is simple
     */
    static std::vector<double> toValuation(TimedCondition condition) {
      std::vector<double> result;
      result.resize(condition.size());
      for (std::size_t i = 0; i < condition.size(); ++i) {
        const auto lowerBound = condition.getLowerBound(i, condition.size() - 1);
        const auto upperBound = condition.getUpperBound(i, condition.size() - 1);
        if (lowerBound.first == -upperBound.first && lowerBound.second && upperBound.second) {
          // When the bound is a point
          result.at(i) = upperBound.first;
        } else {
          // When the bound is not a point
          auto middlePoint = (upperBound.first - lowerBound.first) / 2.0;
          result.at(i) = middlePoint;
          condition.restrictLowerBound(i, condition.size() - 1, Bounds{-middlePoint, true}, false);
          condition.restrictUpperBound(i, condition.size() - 1, Bounds{middlePoint, true}, false);
        }
      }

      return result;
    }

    /*!
     * @brief Add a transition to targetState
     *
     * The transition corresponds to a morphism \f$(u, \Lambda, u', \Lambda', R)\f$, where
     *     - \f$\Lambda\f$ is sourceCondition,
     *     - \f$\Lambda\f$ is targetCondition, and
     *     - \f$R\f$ is renamingRelation.
     */
    void add(const std::shared_ptr<TAState> &targetState, const RenamingRelation &renamingRelation,
             const TimedCondition &sourceCondition, const TimedCondition &targetCondition) {
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
     * @brief Generate transitions
     *
     * @todo There is an issue in reset generation
     * domain: (ab, 1 <= T_{0, 0}  <= 1 && 1 <= T_{0, 1}  <= 1 && 1 < T_{0, 2}  < 2 && -0 <= T_{1, 1}  <= 0 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1)
     * codomain: (ab, -0 < T_{0, 0}  < 1 && 1 <= T_{0, 1}  <= 1 && 1 < T_{0, 2}  < 2 && -0 < T_{1, 1}  < 1 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1)
     * renaming: {t1 == t'1}
     * loc6->loc5 [label="ε", guard="{x0 > 1, x1 > 0, x2 > 0}", reset="{x0 := 1.5, x2 := 0.5}"]
     * The order gets broken
     */
    [[nodiscard]] std::vector<TATransition> make() const {
      std::vector<TATransition> result;
      result.reserve(sourceMap.size());

      for (const auto &[targetWithRenaming, sourceConditions]: sourceMap) {
        const auto &[target, currentRenamingRelation] = targetWithRenaming;
        const auto targetConditions = targetMap.at(targetWithRenaming);
        assert(sourceConditions.size() == targetConditions.size());
        for (std::size_t i = 0; i < sourceConditions.size(); ++i) {
          const auto sourceCondition = sourceConditions.getConditions().at(i);
          const auto targetCondition = targetConditions.getConditions().at(i);
          BOOST_LOG_TRIVIAL(trace) << "Constructing a transition with " << sourceCondition << " and "
                                   << currentRenamingRelation.size();
          BOOST_LOG_TRIVIAL(trace) << "target condition: " << targetCondition;
          // Generate transitions
          auto resets = currentRenamingRelation.toReset(sourceCondition, targetCondition);
          BOOST_LOG_TRIVIAL(trace) << "Resets from renaming: " << resets;
          auto targetValuation = learnta::ExternalTransitionMaker::toValuation(targetCondition);
          // Map the clock variables to the target timed condition if it is not mapped with the renaming relation
          for (auto resetVariable = 0; resetVariable < targetCondition.size(); ++resetVariable) {
            if (resets.end() == std::find_if(resets.begin(), resets.end(), [&](const auto &pair) {
              return pair.first == resetVariable;
            })) {
              resets.emplace_back(resetVariable, targetValuation.at(resetVariable));
            }
          }
          BOOST_LOG_TRIVIAL(trace) << "Resets: " << resets;
          result.emplace_back(target.get(), clean(resets), sourceCondition.toGuard());
        }
      }

      return result;
    }
  };
}

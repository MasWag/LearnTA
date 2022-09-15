/**
 * @author Masaki Waga
 * @date 2022/09/14.
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
   * @brief A class to make a transition from P to P
   */
  class InternalTransitionMaker {
  private:
    // TargetTAState -> SourceTimedCondition
    boost::unordered_map<std::shared_ptr<TAState>, TimedConditionSet> sourceMap;
    // Pairs of \f$\Lambda\f$ and \f$ext^t(\Lambda)\f$ such that \f$\Lambda\f$ is a boundary of P and successor(P)
    std::vector<std::pair<TimedCondition, TimedCondition>> boundaryExteriors;

    /*!
     * @brief Construct a reset to a value in the timed condition
     *
     * @pre condition is simple
     */
    static TATransition::Resets toReset(TimedCondition condition) {
      assert(condition.isSimple());
      BOOST_LOG_TRIVIAL(trace) << "Input condition: " << condition;
      TATransition::Resets result;
      result.reserve(condition.size());
      for (int i = 0; i < condition.size(); ++i) {
        const auto lowerBound = condition.getLowerBound(i, condition.size() - 1);
        const auto upperBound = condition.getUpperBound(i, condition.size() - 1);
        if (lowerBound.first == -upperBound.first && lowerBound.second && upperBound.second) {
          // When the bound is a point
          result.emplace_back(i, upperBound.first);
        } else {
          // When the bound is not a point
          auto middlePoint = (upperBound.first - lowerBound.first) / 2.0;
          result.emplace_back(i, middlePoint);
          condition.restrictLowerBound(i, condition.size() - 1, Bounds{-middlePoint, true});
          condition.restrictUpperBound(i, condition.size() - 1, Bounds{middlePoint, true});
        }
      }

      BOOST_LOG_TRIVIAL(trace) << "Resulting reset: " << result;
      return result;
    }
  public:
    /*!
     * @brief Add a transition to targetState
     *
     * The transition is from \f$(u, \Lambda)\f$, but can also be from the continuous immediate exterior \f$ext^t(\Lambda)\f$, where
     *     - \f$\Lambda\f$ is sourceCondition,
     *     - \f$\Lambda \lor ext^t(\Lambda)\f$ is sourceExternalCondition
     */
    void add(const std::shared_ptr<TAState> &targetState, const TimedCondition &sourceCondition,
             const std::optional<TimedCondition> &sourceExternalConditionOpt = std::nullopt) {
      BOOST_LOG_TRIVIAL(trace) << "sourceCondition: " << sourceCondition;

      if (sourceExternalConditionOpt) {
        BOOST_LOG_TRIVIAL(trace) << "sourceExternalConditionOpt: " << *sourceExternalConditionOpt;
        boundaryExteriors.emplace_back(sourceCondition, *sourceExternalConditionOpt);
      }
      auto it = sourceMap.find(targetState);
      if (it == sourceMap.end()) {
        sourceMap[targetState] = TimedConditionSet{sourceCondition};
      } else {
/*        // TODO: Implement state merging for optimization
        // When there is a transition to targetState, we try to merge the timedCondition
        if (predecessorSourceCondition && !sourceExternalConditionOpt) {
          auto jt = std::find_if(it->second.begin(), it->second.end(),
                                 [&] (const auto &condition) {
            return condition.includes(*predecessorSourceCondition);
          });
          if (jt != it->second.end()) {
            jt->convexHullAssign(sourceCondition);
          } else {
            it->second.push_back(sourceCondition);
          }
        } else {*/
          it->second.push_back(sourceCondition);
        //}
      }
    }

    /*!
     * @brief Generate transitions
     */
    [[nodiscard]] std::vector<TATransition> make() const {
      std::vector<TATransition> result;
      result.reserve(sourceMap.size());

      for (const auto&[target, sourceConditions]: sourceMap) {
        for (const auto &sourceCondition: sourceConditions.getConditions()) {
          const auto it = std::find_if(boundaryExteriors.begin(), boundaryExteriors.end(), [&](const auto pair) {
            return sourceCondition.includes(pair.first);
          });
          if (it == boundaryExteriors.end()) {
            // We only have to refresh the new variable
            const TATransition::Resets resets{std::make_pair(sourceCondition.size(), 0.0)};
            result.emplace_back(target.get(), resets, sourceCondition.toGuard());
          } else {
            assert(sourceCondition == it->first);
            // We project to the non-exterior area and refresh the new variable
            auto resets = toReset(sourceCondition);
            resets.emplace_back(sourceCondition.size(), 0.0);
            result.emplace_back(target.get(), resets, it->second.toGuard());
          }
        }
      }

      return result;
    }

    [[nodiscard]] bool empty() const {
      return this->sourceMap.empty();
    }

    [[nodiscard]] std::size_t size() const {
      return this->sourceMap.size();
    }
  };
}

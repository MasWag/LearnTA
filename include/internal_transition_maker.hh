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

  public:
    /*!
     * @brief Add a transition to targetState
     *
     * The transition is from \f$(u, \Lambda)\f$, where
     *     - \f$\Lambda\f$ is sourceCondition.
     */
    void add(const std::shared_ptr<TAState> &targetState, const TimedCondition &sourceCondition) {
      BOOST_LOG_TRIVIAL(trace) << "sourceCondition: " << sourceCondition;

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

      for (const auto &[target, sourceConditions]: sourceMap) {
        for (const auto &sourceCondition: sourceConditions.getConditions()) {
          // Generate transitions
          // We only have to refresh the new variable
          const TATransition::Resets resets{std::make_pair(sourceCondition.size(), 0.0)};
          result.emplace_back(target.get(), resets, sourceCondition.toGuard());
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

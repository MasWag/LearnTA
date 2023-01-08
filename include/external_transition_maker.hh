/**
 * @author Masaki Waga
 * @date 2022/03/27.
 */

#pragma once

#include <utility>
#include <memory>
#include <numeric>

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
     *     - \f$\Lambda'\f$ is targetCondition, and
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
        BOOST_LOG_TRIVIAL(debug) << "currentRenamingRelation: " << currentRenamingRelation;
        const auto targetConditions = targetMap.at(targetWithRenaming);
        assert(sourceConditions.size() == targetConditions.size());
        for (std::size_t i = 0; i < sourceConditions.size(); ++i) {
          const auto sourceCondition = sourceConditions.getConditions().at(i);
          const auto targetCondition = targetConditions.getConditions().at(i);
          // Add implicit renaming relations
          auto juxtaposedCondition = sourceCondition ^ targetCondition;
          juxtaposedCondition.addRenaming(currentRenamingRelation);
          auto newRenamingRelation = RenamingRelation{juxtaposedCondition.makeRenaming()};
          // Make it unique in terms of the right variables
          std::sort(newRenamingRelation.begin(), newRenamingRelation.end(), [] (const auto &left, const auto& right) {
            return left.second < right.second || (left.second == right.second && left.first < right.first);
          });
          newRenamingRelation.erase(std::unique(newRenamingRelation.begin(), newRenamingRelation.end(),
                                                [] (const auto &left, const auto &right) {
            return left.second == right.second;
          }), newRenamingRelation.end());
          BOOST_LOG_TRIVIAL(debug) << "Constructing a transition with " << sourceCondition << " and "
                                   << newRenamingRelation;
          BOOST_LOG_TRIVIAL(debug) << "target condition: " << targetCondition;
          // Generate transitions
          auto resets = newRenamingRelation.toReset(sourceCondition, targetCondition);
          BOOST_LOG_TRIVIAL(debug) << "Resets from renaming: " << resets;
          auto targetValuation = learnta::ExternalTransitionMaker::toValuation(targetCondition);
          // Map the clock variables to the target timed condition if it is not mapped with the renaming relation
          for (std::size_t resetVariable = 0; resetVariable < targetCondition.size(); ++resetVariable) {
            auto it = std::find_if(resets.begin(), resets.end(), [&](const auto &pair) {
              return pair.first == resetVariable;
            });
            if (it == resets.end()) {
              resets.emplace_back(resetVariable, targetValuation.at(resetVariable));
            } else if (targetValuation.at(resetVariable) == std::floor(targetValuation.at(resetVariable))) {
              // We overwrite the assigned value if it is a constant
              it->second = targetValuation.at(resetVariable);
            }
          }
          BOOST_LOG_TRIVIAL(debug) << "Resets: " << resets;
          result.emplace_back(target.get(), clean(resets), sourceCondition.toGuard());
        }
      }

      return result;
    }

    /*!
     * @brief Return the inactive clock variables after the transition
     */
    static auto inactiveClockVariables(const RenamingRelation &renamingRelation, const TimedCondition &targetCondition) {
      std::vector<ClockVariables> inactiveClocks;
      inactiveClocks.resize(targetCondition.size());
      const ClockVariables lastClock = targetCondition.size() - 1;
      std::iota(inactiveClocks.begin(), inactiveClocks.end(), 0);
      for (const auto &activeClock: renamingRelation.rightVariables()) {
        inactiveClocks.erase(std::remove(inactiveClocks.begin(), inactiveClocks.end(), activeClock), inactiveClocks.end());
      }
      std::unordered_map<ClockVariables, std::size_t> result;
      for (const auto &inactiveClock: inactiveClocks) {
        // Check if the clock variable is active because of a constant constraint
        if (!isPoint(targetCondition.getUpperBound(inactiveClock, lastClock), targetCondition.getLowerBound(inactiveClock, lastClock))) {
          result[inactiveClock] = fabs(targetCondition.getUpperBound(inactiveClock, lastClock).first);
        }
      }
      BOOST_LOG_TRIVIAL(debug) << "inactiveClockVariables: " << renamingRelation << ", " << targetCondition;
      for (const auto &[inactiveClock, bound]: result) {
        BOOST_LOG_TRIVIAL(debug) << "inactiveClockVariables: " << int(inactiveClock) << ", " << bound;
      }

      return result;
    }
  };
}

namespace std {
  static inline std::ostream &operator<<(std::ostream &os, const std::pair<learnta::ClockVariables, std::size_t> &inactiveClockPair) {
    const auto &[inactiveClock, bound] = inactiveClockPair;
    os << "x" << int(inactiveClock) << ", " << bound;

    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const std::unordered_map<learnta::ClockVariables, std::size_t> &inactiveClocks) {
    bool isFirst = true;
    for (const auto &[inactiveClock, bound]: inactiveClocks) {
      if (!isFirst) {
        os << ", ";
      }
      os << "x" << int(inactiveClock) << ", " << bound;
      isFirst = false;
    }

    return os;
  }
}

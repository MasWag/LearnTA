/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include <utility>
#include <vector>
#include <queue>
#include <unordered_map>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

#include "forward_regional_elementary_language.hh"
#include "backward_regional_elementary_language.hh"
#include "symbolic_membership_oracle.hh"
#include "timed_automaton.hh"
#include "equivalence.hh"
#include "renaming_relation.hh"

namespace learnta {
  /*!
   * @brief Timed observation table
   */
  class ObservationTable {
  private:
    std::unique_ptr<SymbolicMembershipOracle> memOracle;
    std::vector<Alphabet> alphabet;
    std::vector<ForwardRegionalElementaryLanguage> prefixes;
    std::vector<BackwardRegionalElementaryLanguage> suffixes;
    // The indexes of prefixes in P
    std::unordered_set<std::size_t> pIndices;
    // prefixes[first] and prefixes[second.first] are in the same equivalence class witnessed by second.second
    std::unordered_map<std::size_t, std::unordered_map<std::size_t, RenamingRelation>> closedRelation;
    // The table containing the symbolic membership
    std::vector<std::vector<TimedConditionSet>> table;
    std::unordered_map<std::size_t, std::size_t> continuousSuccessors;
    boost::unordered_map<std::pair<std::size_t, Alphabet>, std::size_t> discreteSuccessors;
    // The pair of prefixes such that we know that they are distinguished
    boost::unordered_set<std::pair<std::size_t, std::size_t>> distinguishedPrefix;

    // Fill the observation table
    void refreshTable() {
      table.resize(prefixes.size());
      for (int prefixIndex = 0; prefixIndex < prefixes.size(); ++prefixIndex) {
        const auto originalSize = table.at(prefixIndex).size();
        table.at(prefixIndex).resize(suffixes.size());
        for (auto suffixIndex = originalSize; suffixIndex < suffixes.size(); ++suffixIndex) {
          table.at(prefixIndex).at(suffixIndex) = this->memOracle->query(
                  prefixes.at(prefixIndex) + suffixes.at(suffixIndex));
        }
      }
    }

    void moveToP(std::size_t index) {
      // index should not be in P yet
      assert(pIndices.find(index) == pIndices.end());
      pIndices.insert(index);
      // Add successors to the prefixes
      prefixes.reserve(prefixes.size() + alphabet.size() + 1);
      for (Alphabet c: alphabet) {
        discreteSuccessors[std::make_pair(index, c)] = prefixes.size();
        prefixes.push_back(prefixes.at(index).successor(c));
      }
      continuousSuccessors[index] = prefixes.size();
      prefixes.push_back(prefixes.at(index).successor());
      // fill the observation table
      refreshTable();
    }

    bool equivalent(std::size_t i, std::size_t j) {
      auto renamingRelation = findEquivalentRenaming(this->prefixes.at(i), this->table.at(i),
                                                     this->prefixes.at(j), this->table.at(j), this->suffixes);
      if (renamingRelation) {
        this->closedRelation[i][j] = renamingRelation.value();
        return true;
      } else {
        this->distinguishedPrefix.insert(std::make_pair(i, j));
        return false;
      }
    }

    bool equivalentWithMemo(std::size_t i, std::size_t j) {
      if (this->distinguishedPrefix.find(std::make_pair(i, j)) != this->distinguishedPrefix.end() ||
          this->distinguishedPrefix.find(std::make_pair(j, i)) != this->distinguishedPrefix.end()) {
        // we already know that they are not equivalent
        assert(!this->equivalent(i, j));
        return false;
      }
      auto equivalentPrefixesIT = this->closedRelation.find(i);
      if (equivalentPrefixesIT != this->closedRelation.end()) {
        auto it = equivalentPrefixesIT->second.find(j);
        if (it != this->closedRelation.at(i).end()) {
          if (equivalence(this->prefixes.at(i), this->table.at(i),
                          this->prefixes.at(j), this->table.at(j), this->suffixes, it->second)) {
            return true;
          }
        }
      }
      return this->equivalent(i, j);
    }

    [[nodiscard]] bool
    equivalent(std::size_t i, std::size_t j, const BackwardRegionalElementaryLanguage &newSuffix) const {
      auto leftRow = this->table.at(i);
      leftRow.push_back(this->memOracle->query(prefixes.at(i) + newSuffix));
      auto rightRow = this->table.at(j);
      rightRow.push_back(this->memOracle->query(prefixes.at(j) + newSuffix));
      auto newSuffixes = this->suffixes;
      newSuffixes.push_back(newSuffix);

      return findEquivalentRenaming(this->prefixes.at(i), leftRow, this->prefixes.at(j), rightRow,
                                    newSuffixes).has_value();
    }

    /*!
     * @brief Returns if row[i] is accepting or not
     */
    [[nodiscard]] bool isMatch(std::size_t i) const {
      return !this->table.at(i).at(0).empty();
    }

    /*!
     * @brief Returns if prefixes[i] is in P or not
     */
    [[nodiscard]] bool inP(std::size_t i) const {
      return this->pIndices.find(i) != this->pIndices.end();
    }

  public:
    /*!
     * @brief Initialize the observation table
     */
    ObservationTable(std::vector<Alphabet> alphabet, std::unique_ptr<SymbolicMembershipOracle> memOracle) :
            memOracle(std::move(memOracle)),
            alphabet(std::move(alphabet)),
            prefixes{ForwardRegionalElementaryLanguage{}},
            suffixes{BackwardRegionalElementaryLanguage{}} {
      this->moveToP(0);
      this->refreshTable();
    }

    /*!
     * @brief Close the observation table
     *
     * @returns returns true if the observation table is already closed
     */
    bool close() {
      for (std::size_t i = 0; i < this->prefixes.size(); i++) {
        {
          // Check if this prefix is in P
          auto it = this->pIndices.find(i);
          if (it != this->pIndices.end()) {
            continue;
          }
        }
        auto prefix = this->prefixes.at(i);
        auto prefixRow = this->table.at(i);
        bool found = false;
        {
          auto it = this->closedRelation.find(i);
          if (it != closedRelation.end()) {
            // When we already know that this prefix is equivalent to one of p \in P, we just confirm it.
            for (auto targetIt = it->second.begin(); targetIt != it->second.end();) {
              auto renamingRelation = targetIt->second;
              if (equivalence(prefix, prefixRow,
                              this->prefixes.at(targetIt->first), this->table.at(targetIt->first), this->suffixes,
                              renamingRelation)) {
                found = true;
                break;
              } else {
                targetIt = it->second.erase(targetIt);
              }
            }
          }
        }
        // We have no idea of the target prefix, and we find an equivalent row
        if (!found) {
          found = std::any_of(this->pIndices.begin(), this->pIndices.end(), [&](const auto j) {
            return equivalent(i, j);
          });
        }
        if (!found) {
          // The observation table is not closed
          const auto prePSize = this->pIndices.size();
          this->moveToP(i);
          assert(prePSize < this->pIndices.size());
          BOOST_LOG_TRIVIAL(debug) << "Observation table is not closed because of " << this->prefixes.at(i);
          return false;
        }
      }

      return true;
    }

    /*!
     * @brief Make the observation table consistent
     *
     * @returns This return true if the observation table is already consistent
     */
    bool consistent() {
      for (const auto i: pIndices) {
        for (const auto j: pIndices) {
          bool eq = false;
          if (i <= j) {
            continue;
          }
          if (this->equivalentWithMemo(i, j)) {
            // confirm the consistency
            for (const auto action: this->alphabet) {
              if (!this->equivalentWithMemo(this->discreteSuccessors.at({i, action}),
                                            this->discreteSuccessors.at({j, action}))) {
                BOOST_LOG_TRIVIAL(debug) << "Observation table is inconsistent because of discrete successor of "
                                         << this->prefixes.at(i) << " and " << this->prefixes.at(j)
                                         << " with action " << action;

                // The observation table is inconsistent due to a discrete successor
                auto it = std::find_if(suffixes.begin(), suffixes.end(), [&](const auto &suffix) {
                  return !equivalent(i, j, suffix.predecessor(action));
                });
                // we assume that we have such a suffix
                const auto newSuffix = it->predecessor(action);
                suffixes.push_back(newSuffix);
                BOOST_LOG_TRIVIAL(debug) << "New suffix " << newSuffix << " is added";

                this->refreshTable();
                // i and j should not be equivalent after adding the new suffix
                assert(!this->equivalentWithMemo(i, j));

                return false;
              }
            }
            if (!this->equivalentWithMemo(this->continuousSuccessors.at(i), this->continuousSuccessors.at(j))) {
              // The observation table is inconsistent due to a continuous successor
              auto it = std::find_if(suffixes.begin(), suffixes.end(), [&](const auto &suffix) {
                return !equivalent(i, j, suffix.predecessor());
              });
              // we assume that we have such a suffix
              suffixes.push_back(it->predecessor());
              this->refreshTable();

              return false;
            }
          }
        }
      }
      return true;
    }

    /*!
     * @brief Make the observation table exterior-consistent
     *
     * @returns if the observation table is already exterior-consistent
     */
    bool exteriorConsistent() {
      std::vector<std::size_t> newP;
      newP.reserve(pIndices.size());
      for (const std::size_t pIndex: pIndices) {
        const auto successorIndex = continuousSuccessors.at(pIndex);
        // Check if p is the boundary
        if (this->inP(successorIndex)) {
          continue;
        }
        // Check if p has equality constraints in \f$\mathbb{T}_{i,N}\f$.
        if (prefixes.at(pIndex).hasEqualityN()) {
          continue;
        }
        newP.push_back(successorIndex);
      }
      if (newP.empty()) {
        return true;
      }

      // update the observation table
      for (auto newIndex: newP) {
        this->moveToP(newIndex);
      }
      this->refreshTable();
      return false;
    }

    /*!
     * @brief Add each prefix of counterExample to P
     */
    void addCounterExample(const ForwardRegionalElementaryLanguage &counterExample) {
      const auto newPrefixes = counterExample.prefixes();
      for (const auto &prefix: newPrefixes) {
        auto it = std::find(this->prefixes.begin(), this->prefixes.end(), prefix);
        // prefix should be in the observation table
        assert(it != this->prefixes.end());
        const auto index = it - this->prefixes.begin();
        auto pIt = this->pIndices.find(index);
        if (pIt == this->pIndices.end()) {
          this->moveToP(index);
        }
      }
    }

    /*!
     * @brief Construct a hypothesis DTA from the current timed observation table
     *
     * @pre The observation table is closed, consistent, and exterior-consistent.
     * @todo We currently construct only the DTAs without unobservable transitions.
     */
    TimedAutomaton generateHypothesis() {
      std::unordered_map<std::size_t, std::shared_ptr<TAState>> indexToState;
      std::unordered_map<std::shared_ptr<TAState>, std::vector<std::size_t>> stateToIndices;
      std::vector<std::shared_ptr<TAState>> states;
      auto addState = [&](std::size_t index) {
        auto state = std::make_shared<TAState>(this->isMatch(index));
        indexToState[index] = state;
        auto it = stateToIndices.find(state);
        if (it == stateToIndices.end()) {
          stateToIndices[state] = {index};
        } else {
          it->second.push_back(index);
        }
        states.push_back(state);

        return state;
      };

      auto initialState = addState(0);
      // construct the initial state
      const auto handleInternalContinuousSuccessors = [&](const std::size_t initialIndex) {
        auto sourceIndex = initialIndex;
        auto nextIndex = this->continuousSuccessors[initialIndex];
        while (this->inP(nextIndex)) {
          if (isMatch(sourceIndex) == isMatch(nextIndex)) {
            auto state = indexToState[sourceIndex];
            indexToState[nextIndex] = state;
            stateToIndices[state].push_back(nextIndex);
          } else {
            // We have not implemented such a case that an unobservable transition is necessary
            abort();
          }
          sourceIndex = nextIndex;
          nextIndex = this->continuousSuccessors[nextIndex];
        }

        // Our optimization to merge the continuous exterior
        if (equivalentWithMemo(nextIndex, sourceIndex)) {
          auto state = indexToState[sourceIndex];
          indexToState[nextIndex] = state;
          stateToIndices[state].push_back(nextIndex);
        }
      };
      handleInternalContinuousSuccessors(0);

      std::vector<std::pair<std::size_t, Alphabet>> discreteBoundaries;

      // explore new states
      std::queue<std::shared_ptr<TAState>> newStates;
      newStates.push(initialState);
      while (!newStates.empty()) {
        auto newState = newStates.front();
        newStates.pop();
        auto newStateIndices = stateToIndices.at(newState);
        for (const auto action: alphabet) {
          // TargetState -> the timed condition to reach that state
          std::unordered_map<std::shared_ptr<TAState>, TimedCondition> sourceMap;
          for (auto it = newStateIndices.begin(); it != newStateIndices.end(); ++it) {
            if (this->discreteSuccessors.find(std::make_pair(*it, action)) == this->discreteSuccessors.end()) {
              // if `it` is not in P
              // q' in the diagram below.
              const auto discrete = this->discreteSuccessors[std::make_pair(*std::prev(it), action)];
              // let the discrete successor of *it to q' if it and std::prev(it) are equivalent
              if (this->inP(discrete) && this->equivalentWithMemo(*it, *std::prev(it))) {
                sourceMap[indexToState.at(discrete)].removeEqualityUpperBoundAssign();
              }

              continue;
            }
            const auto discreteSuccessorIndex = this->discreteSuccessors.at(std::make_pair(*it, action));
            // Add states only if the successor is also in P
            if (!this->inP(discreteSuccessorIndex)) {
              discreteBoundaries.emplace_back(newStateIndices.front(), action);
              continue;
            }
            /*
             * Check if the following q and q'' are similar. If yes, we merge q with q'
             *          std::prev(it) ---cont--> it
             *           |                        |
             *         action                   action
             *           |                        |
             *           v                        v
             *           q'--------cont--> q'' ~~ q
             */
            // we just add a new state if std::prev(it) does not exist or q' in the above diagram is not in P
            if (it == newStateIndices.begin() ||
                !this->inP(this->discreteSuccessors[std::make_pair(*std::prev(it), action)])) {
              auto successor = addState(discreteSuccessorIndex);
              newStates.push(successor);
              handleInternalContinuousSuccessors(discreteSuccessorIndex);
              sourceMap[successor] = this->prefixes.at(*it).getTimedCondition();
            } else {
              // q in the above diagram
              const auto discreteAfterContinuous =
                      this->discreteSuccessors[std::make_pair(*it, action)];
              // q' in the above diagram
              const auto discrete = this->discreteSuccessors[std::make_pair(*std::prev(it), action)];
              // q'' in the above diagram
              const auto continuousAfterDiscrete = this->continuousSuccessors[discrete];

              if (this->equivalentWithMemo(continuousAfterDiscrete, discreteAfterContinuous)) {
                // merge q and q'' in the above diagram
                indexToState.at(discreteAfterContinuous) = indexToState.at(discrete);
                stateToIndices[indexToState.at(discrete)].push_back(discreteAfterContinuous);

                // We use the convexity of the timed conditions of this and its continuous successor
                sourceMap[indexToState.at(discrete)] = sourceMap[indexToState.at(discrete)].convexHull(
                        this->prefixes.at(*it).getTimedCondition());
              } else {
                // Otherwise, we create a new state
                const auto successor = addState(discreteAfterContinuous);
                newStates.push(successor);
                sourceMap[successor] = this->prefixes.at(*it).getTimedCondition();
              }

              handleInternalContinuousSuccessors(discreteAfterContinuous);
            }
          }

          newState->next[action].reserve(sourceMap.size());
          for (const auto&[target, timedCondition]: sourceMap) {
            newState->next[action].emplace_back(target.get(),
                                                std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>>{
                                                        std::make_pair(timedCondition.size(), std::nullopt)},
                                                timedCondition.toGuard());
          }
        }
      }

      //! Construct transitions by discrete immediate exteriors
      std::sort(discreteBoundaries.begin(), discreteBoundaries.end());
      discreteBoundaries.erase(std::unique(discreteBoundaries.begin(), discreteBoundaries.end()),
                               discreteBoundaries.end());
      for (auto[sourceIndex, action]: discreteBoundaries) {
        const auto originalSourceIndex = sourceIndex;

        // (TargetState, RenamingRelation) -> TimedCondition of the source
        boost::unordered_map<std::pair<std::shared_ptr<TAState>, RenamingRelation>, TimedConditionSet> sourceMap;
        auto targetIndex = this->discreteSuccessors.at(std::make_pair(sourceIndex, action));
        unsigned long jumpedTargetIndex;
        RenamingRelation renamingRelation;
        {
          assert(this->pIndices.find(targetIndex) == this->pIndices.end());
          // Find a successor in P
          auto it = std::find_if(this->closedRelation.at(targetIndex).begin(),
                                 this->closedRelation.at(targetIndex).end(), [&](const auto &rel) {
                    return this->inP(rel.first);
                  });
          jumpedTargetIndex = it->first;
          renamingRelation = it->second;
          sourceMap[std::make_pair(indexToState.at(jumpedTargetIndex), renamingRelation)] =
                  TimedConditionSet{this->prefixes.at(sourceIndex).getTimedCondition()};
        }

        while (this->continuousSuccessors.find(sourceIndex) != this->continuousSuccessors.end()) {
          sourceIndex = this->continuousSuccessors.at(sourceIndex);
          BOOST_LOG_TRIVIAL(debug) << "Constructing a transition from " << this->prefixes.at(sourceIndex);
          {
            auto it = this->discreteSuccessors.find(std::make_pair(sourceIndex, action));
            if (it == this->discreteSuccessors.end()) {
              // Include immediate exterior
              sourceMap[std::make_pair(indexToState.at(jumpedTargetIndex),
                                       renamingRelation)].removeEqualityUpperBoundAssign();
              continue;
            }
            targetIndex = it->second;
            if (this->pIndices.find(targetIndex) != this->pIndices.end()) {
              break;
            }
          }
          // Find a successor in P
          auto it = std::find_if(this->closedRelation.at(targetIndex).begin(),
                                 this->closedRelation.at(targetIndex).end(), [&](const auto &rel) {
                    return this->inP(rel.first);
                  });
          jumpedTargetIndex = it->first;
          assert(this->inP(jumpedTargetIndex));
          renamingRelation = it->second;

          {
            auto it2 = sourceMap.find(std::make_pair(indexToState.at(jumpedTargetIndex), renamingRelation));
            if (it2 == sourceMap.end()) {
              sourceMap[std::make_pair(indexToState.at(jumpedTargetIndex), renamingRelation)] =
                      TimedConditionSet{this->prefixes.at(sourceIndex).getTimedCondition()};
            } else {
              if (it2->second.back().includes(this->prefixes.at(sourceIndex).immediatePrefix()->getTimedCondition())) {
                it2->second.back().convexHullAssign(this->prefixes.at(sourceIndex).getTimedCondition());
              } else {
                it2->second.push_back(this->prefixes.at(sourceIndex).getTimedCondition());
              }
            }
          }
        }

        indexToState[originalSourceIndex]->next[action].reserve(sourceMap.size());
        for (auto&[targetWithRenaming, timedCondition]: sourceMap) {
          const auto &[target, currentRenamingRelation] = targetWithRenaming;
          for (const auto &condition: timedCondition.getConditions()) {
            BOOST_LOG_TRIVIAL(debug) << "Constructing a transition with " << condition << " and "
                                     << currentRenamingRelation.size();
            indexToState[originalSourceIndex]->next[action].emplace_back(target.get(),
                                                                         currentRenamingRelation.toReset(),
                                                                         condition.toGuard());
          }
        }
      }

      //! @todo We need to implement transitions by continuous immediate exteriors to support unobservable transitions

      // Construct the max constants
      std::vector<int> maxConstants;
      for (const auto &state: states) {
        for (const auto &[action, transitions]: state->next) {
          for (const auto &transition: transitions) {
            for (const auto &guard: transition.guard) {
              if (guard.x >= maxConstants.size()) {
                maxConstants.resize(guard.x + 1);
              }
              maxConstants[guard.x] = std::max(maxConstants[guard.x], guard.c);
            }
          }
        }
      }

      return TimedAutomaton{{states, {initialState}}, maxConstants};
    }

    std::ostream &printDetail(std::ostream &stream) const {
      printStatistics(stream);
      stream << "P is as follows\n";
      for (const auto &pIndex: this->pIndices) {
        stream << this->prefixes.at(pIndex) << "\n";
      }
      stream << "S is as follows\n";
      for (const auto &suffix: this->suffixes) {
        stream << suffix << "\n";
      }

      return stream;
    }

    std::ostream &printStatistics(std::ostream &stream) const {
      stream << "|P| = " << this->pIndices.size() << "\n";
      stream << "|ext(P)| = " << this->prefixes.size() - this->pIndices.size() << "\n";
      stream << "|S| = " << this->suffixes.size() << "\n";

      return stream;
    }
  };
}
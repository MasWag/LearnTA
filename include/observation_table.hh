/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include <utility>
#include <vector>
#include <queue>
#include <unordered_map>

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
    std::unordered_map<std::pair<std::size_t, Alphabet>, std::size_t> discreteSuccessors;
    // The pair of prefixes such that we know that they are distinguished
    std::unordered_set<std::pair<std::size_t, std::size_t>> distinguishedPrefix;

    // Fill the observation table
    void refreshTable() {
      table.reserve(prefixes.size());
      for (int prefixIndex = 0; prefixIndex < prefixes.size(); ++prefixIndex) {
        if (table.size() >= prefixIndex) {
          table.emplace_back();
        }
        table.at(prefixIndex).resize(suffixes.size());
        for (auto suffixIndex = table.at(prefixIndex).size(); suffixIndex < suffixes.size(); ++suffixIndex) {
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
        prefixes.push_back(prefixes.at(index).successor(c));
        discreteSuccessors.at(std::make_pair(index, c)) = prefixes.size();
      }
      prefixes.push_back(prefixes.at(index).successor());
      continuousSuccessors.at(index) = prefixes.size();
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
        return false;
      }
      auto it = this->closedRelation.at(i).find(j);
      if (it != this->closedRelation.at(i).end()) {
        if (equivalence(this->prefixes.at(i), this->table.at(i),
                        this->prefixes.at(j), this->table.at(j), this->suffixes, it->second)) {
          return true;
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

  public:
    /*!
     * @brief Initialize the observation table
     */
    ObservationTable(std::vector<Alphabet> alphabet, std::unique_ptr<SymbolicMembershipOracle> memOracle) :
            memOracle(std::move(memOracle)),
            alphabet(std::move(alphabet)),
            prefixes{ForwardRegionalElementaryLanguage{}},
            suffixes{BackwardRegionalElementaryLanguage{}} {
      pIndices.insert(0);
      this->moveToP(0);
      this->refreshTable();
    }

    /*!
     * @brief Close the observation table
     *
     * @returns This returns true if the observation table is already closed
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
                targetIt = this->closedRelation.at(i).erase(targetIt);
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
          this->moveToP(i);
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
                // The observation table is inconsistent due to a discrete successor
                auto it = std::find(suffixes.begin(), suffixes.end(), [&](const auto &suffix) {
                  return !equivalent(i, j, suffix.predecessor(action));
                });
                // we assume that we have such a suffix
                suffixes.push_back(it->predecessor(action));

                return false;
              }
            }
            if (!this->equivalentWithMemo(this->continuousSuccessors.at(i), this->continuousSuccessors.at(j))) {
              // The observation table is inconsistent due to a continuous successor
              auto it = std::find(suffixes.begin(), suffixes.end(), [&](const auto &suffix) {
                return !equivalent(i, j, suffix.predecessor());
              });
              // we assume that we have such a suffix
              suffixes.push_back(it->predecessor());

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
        auto it = pIndices.find(successorIndex);
        if (it != pIndices.end()) {
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
     * @todo not implemented
     */
    TimedAutomaton generateHypothesis() {
      std::unordered_map<std::size_t, std::shared_ptr<TAState>> indexToState;
      std::unordered_map<std::shared_ptr<TAState>, std::vector<std::size_t>> stateToIndices;
      std::vector<std::shared_ptr<TAState>> states;
      std::size_t variableSize = 0;
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
      const auto handleInternalContinuousSuccessors = [&](std::size_t initialIndex) {
        auto nextIndex = this->continuousSuccessors[initialIndex];
        while (pIndices.find(nextIndex) != pIndices.end()) {
          if (isMatch(initialIndex) == isMatch(nextIndex)) {
            auto state = indexToState[initialIndex];
            indexToState[nextIndex] = state;
            stateToIndices[state].push_back(nextIndex);
          } else {
            // We have not implemented such a case that an unobservatble transition is necessary
            abort();
          }

          // Our optimization to merge the continuous exterior
          if (equivalentWithMemo(nextIndex, initialIndex)) {
            auto state = indexToState[initialIndex];
            indexToState[nextIndex] = state;
            stateToIndices[state].push_back(nextIndex);
          }

          initialIndex = nextIndex;
          nextIndex = this->continuousSuccessors[initialIndex];
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
          const auto baseSuccessorIndex = this->discreteSuccessors.at(std::make_pair(newStateIndices.front(), action));
          // Add states only if the successor is also in P
          if (this->pIndices.find(baseSuccessorIndex) == this->pIndices.end()) {
            discreteBoundaries.emplace_back(newStateIndices.front(), action);
            continue;
          }

          auto successor = addState(baseSuccessorIndex);
          newStates.push(successor);
          handleInternalContinuousSuccessors(baseSuccessorIndex);
          std::unordered_map<std::shared_ptr<TAState>, TimedCondition> sourceMap;
          sourceMap[successor] = this->prefixes.at(baseSuccessorIndex).getTimedCondition();
          variableSize = std::max(variableSize, sourceMap[successor].size());

          for (auto it = newStateIndices.begin(); std::next(it) != newStateIndices.end(); ++it) {
            const auto discreteAfterContinuous = this->discreteSuccessors[{*std::next(it), action}];
            const auto continuousAfterDiscrete =
                    this->continuousSuccessors[this->discreteSuccessors[std::make_pair(*it, action)]];

            if (this->equivalentWithMemo(continuousAfterDiscrete, discreteAfterContinuous)) {
              // merge p --continuous--> --discrete--> and p --discrete--> --continuous--> if they are equivalent
              indexToState.at(discreteAfterContinuous) = successor;
              stateToIndices[successor].push_back(discreteAfterContinuous);

              // We use the convexity of the timed conditions of this and its continuous successor
              sourceMap[successor] = sourceMap[successor].convexHull(
                      this->prefixes.at(*std::next(it)).getTimedCondition());
            } else {
              // Otherwise, we create a new state
              successor = addState(discreteAfterContinuous);
              newStates.push(successor);
              sourceMap[successor] = this->prefixes.at(*std::next(it)).getTimedCondition();
            }

            handleInternalContinuousSuccessors(discreteAfterContinuous);
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
      for (auto[sourceIndex, action]: discreteBoundaries) {
        std::unordered_map<std::pair<std::shared_ptr<TAState>, RenamingRelation>, TimedCondition> sourceMap;
        auto targetIndex = this->discreteSuccessors.at(std::make_pair(sourceIndex, action));
        {
          auto it = this->closedRelation.at(targetIndex).begin();
          // Find a successor in P
          while (this->pIndices.find(it->first) == this->pIndices.end()) {
            ++it;
          }
          const auto &[jumpedTargetIndex, renamingRelation] = *it;
          sourceMap[std::make_pair(indexToState.at(jumpedTargetIndex), renamingRelation)] =
                  this->prefixes.at(targetIndex).getTimedCondition();
        }

        while (this->continuousSuccessors.find(sourceIndex) != this->continuousSuccessors.end()) {
          sourceIndex = this->continuousSuccessors.at(sourceIndex);
          targetIndex = this->discreteSuccessors.at(std::make_pair(sourceIndex, action));
          auto it = this->closedRelation.at(targetIndex).begin();
          // Find a successor in P
          while (this->pIndices.find(it->first) == this->pIndices.end()) {
            ++it;
          }
          const auto &[jumpedTargetIndex, renamingRelation] = *it;
          sourceMap[std::make_pair(indexToState.at(jumpedTargetIndex), renamingRelation)] =
                  this->prefixes.at(targetIndex).getTimedCondition();
        }
      }

      //! @todo construct transitions by continuous immediate exteriors

      // Construct the max constants
      std::vector<int> maxConstants;
      maxConstants.resize(variableSize);
      for (const auto &state: states) {
        for (const auto &[action, transitions]: state->next) {
          for (const auto &transition: transitions) {
            for (const auto &guard: transition.guard) {
              maxConstants[guard.x] = std::max(maxConstants[guard.x], guard.c);
            }
          }
        }
      }

      return TimedAutomaton{{states, {initialState}}, maxConstants};
    }

    std::ostream &printStatistics(std::ostream &stream) {
      stream << "|P| = " << this->pIndices.size() << "\n";
      stream << "|ext(P)| = " << this->prefixes.size() - this->pIndices.size() << "\n";
      stream << "|S| = " << this->suffixes.size() << "\n";

      return stream;
    }
  };
}
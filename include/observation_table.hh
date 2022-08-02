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
#include "external_transition_maker.hh"

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

    /*!
     * @brief Fill the observation table
     *
     * @post The observation table is filled
     */
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

    /*!
     * @brief Move an index pointing ext(P) to P
     *
     * @param index An index pointing a row in ext(P)
     *
     * @pre index should not point P
     * @pre index should point ext(P)
     * @invariant The observation table is filled
     * @post index should point P
     * @post The discrete and continuous successors of prefixes.at(index) should be in ext(P)
     */
    void moveToP(const std::size_t index) {
      // index should not be in P yet
      assert(pIndices.find(index) == pIndices.end());
      // The index should be in a valid range
      assert(0 <= index && index < prefixes.size());
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
      // index should point P
      assert(pIndices.find(index) != pIndices.end());
      // the discrete successors of prefixes.at(index) should be in ext(P)
      assert(std::all_of(alphabet.begin(), alphabet.end(), [&](Alphabet c) {
        return 0 <= discreteSuccessors.at(std::make_pair(index, c)) &&
          discreteSuccessors.at(std::make_pair(index, c)) < prefixes.size();
      }));
      assert(std::all_of(alphabet.begin(), alphabet.end(), [&](Alphabet c) {
        return pIndices.find(discreteSuccessors.at(std::make_pair(index, c))) == pIndices.end();
      }));
      assert(pIndices.find(index) != pIndices.end());
      // the continuous successor of prefixes.at(index) should be in ext(P)
      assert(0 <= continuousSuccessors.at(index) && continuousSuccessors.at(index) < prefixes.size());
      assert(pIndices.find(continuousSuccessors.at(index)) == pIndices.end());
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

    [[nodiscard]] bool
    equivalent(std::size_t i, std::size_t j, const std::list<BackwardRegionalElementaryLanguage> &newSuffixes) const {
      auto leftRow = this->table.at(i);
      auto rightRow = this->table.at(j);
      auto tmpSuffixes = this->suffixes;
      for (const auto &newSuffix: newSuffixes) {
        leftRow.push_back(this->memOracle->query(prefixes.at(i) + newSuffix));
        rightRow.push_back(this->memOracle->query(prefixes.at(j) + newSuffix));
        tmpSuffixes.push_back(newSuffix);
      }

      return findEquivalentRenaming(this->prefixes.at(i), leftRow, this->prefixes.at(j), rightRow,
                                    tmpSuffixes).has_value();
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
     * The observation table is closed if for any row in ext(P), there is an equivalent row in P.
     *
     * @returns returns true if the observation table is already closed
     */
    bool close() {
      for (std::size_t i = 0; i < this->prefixes.size(); i++) {
        // Skip if this prefix is in P
        if (this->pIndices.find(i) != this->pIndices.end()) {
          continue;
        }
        auto prefix = this->prefixes.at(i);
        auto prefixRow = this->table.at(i);

        // Use the memoized information for efficiency
        bool found = false;
        {
          auto it = this->closedRelation.find(i);
          if (it != closedRelation.end()) {
            // When we already know that this prefix is equivalent to one of p \in P, we just confirm it.
            for (auto targetIt = it->second.begin(); targetIt != it->second.end();) {
              auto renamingRelation = targetIt->second;
              if (equivalence(prefix, prefixRow,
                              this->prefixes.at(targetIt->first), this->table.at(targetIt->first),
                              this->suffixes, renamingRelation)) {
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
#ifndef NDEBUG
          const auto prePSize = this->pIndices.size();
#endif
          this->moveToP(i);
          assert(prePSize < this->pIndices.size());
          BOOST_LOG_TRIVIAL(debug) << "Observation table is not closed because of " << this->prefixes.at(i);
          return false;
        }
      }

      return true;
    }

  private:
    /*!
     * @brief Resolve an inconsistency due to discrete successors
     *
     * @pre equivalent(i, j)
     * @pre !equivalent(discreteSuccessors(i, action), discreteSuccessors(j, action))
     * @post !equivalent(i, j)
     */
    void resolveDiscreteInconsistency(std::size_t i, std::size_t j, Alphabet action) {
      // Find a single witness of the inconsistency
      auto it = std::find_if_not(suffixes.begin(), suffixes.end(), [&](const auto &suffix) {
        return equivalent(i, j, suffix.predecessor(action));
      });
      // we assume that we have such a suffix
      if (it == suffixes.end()) {
        abort();
      }
      const auto newSuffix = it->predecessor(action);
      BOOST_LOG_TRIVIAL(debug) << "New suffix " << newSuffix << " is added";
      suffixes.push_back(newSuffix);

      this->refreshTable();
      // i and j should not be equivalent after adding the new suffix
      assert(!this->equivalentWithMemo(i, j));
    }

    /*!
     * @brief Resolve an inconsistency due to continuous successors
     *
     * @pre equivalent(i, j)
     * @pre !equivalent(continuousSuccessors(i), continuousSuccessors(j))
     * @post !equivalent(i, j)
     */
    void resolveContinuousInconsistency(const std::size_t i, const std::size_t j) {
      // Find a single witness of the inconsistency
      auto it = std::find_if_not(suffixes.begin(), suffixes.end(), [&](const auto &suffix) {
        return equivalent(i, j, suffix.predecessor());
      });
      if (it != suffixes.end()) {
        const auto newSuffix = it->predecessor();
        BOOST_LOG_TRIVIAL(debug) << "New suffix " << newSuffix << " is added";
        suffixes.push_back(newSuffix);
      } else {
        // We find a set of suffixes to add
        std::list<BackwardRegionalElementaryLanguage> allPredecessors;
        allPredecessors.resize(suffixes.size());
        std::transform(suffixes.begin(), suffixes.end(), allPredecessors.begin(), [](const auto &suffix) {
          return suffix.predecessor();
        });
        if (equivalent(i, j, allPredecessors)) {
          BOOST_LOG_TRIVIAL(info) << "Finding longer predecessors. This is slow";
          auto preAllPredecessors = allPredecessors;
          do {
            decltype(allPredecessors) newAllPredecessors;
            for (const auto &suffix: preAllPredecessors) {
              try {
                newAllPredecessors.push_back(suffix.predecessor());
              } catch (...) {
              }
            }
            if (newAllPredecessors.empty()) {
              BOOST_LOG_TRIVIAL(fatal) << "Something is wrong in resolving continuous inconsistency";
              abort();
            }
            preAllPredecessors = std::move(newAllPredecessors);
            std::copy(preAllPredecessors.begin(), preAllPredecessors.end(), std::back_inserter(allPredecessors));
          } while (equivalent(i, j, allPredecessors));
        }
        while (!allPredecessors.empty()) {
          auto jt = std::find_if_not(allPredecessors.begin(), allPredecessors.end(), [&](const auto &suffix) {
            auto examinedPredecessor = allPredecessors;
            auto kt = std::find(examinedPredecessor.begin(), examinedPredecessor.end(), suffix);
            examinedPredecessor.erase(kt);
            return equivalent(i, j, examinedPredecessor);
          });
          if (jt == allPredecessors.end()) {
            break;
          }
          allPredecessors.erase(jt);
        }

        suffixes.reserve(suffixes.size() + allPredecessors.size());
        std::move(allPredecessors.begin(), allPredecessors.end(), std::back_inserter(suffixes));
      }

      this->refreshTable();
      // i and j should not be equivalent after adding the new suffix
      assert(!this->equivalentWithMemo(i, j));
    }
  public:
    /*!
     * @brief Make the observation table consistent
     *
     * @returns true if the observation table is already consistent
     */
    bool consistent() {
      for (const auto i: pIndices) {
        for (const auto j: pIndices) {
          if (i <= j) {
            continue;
          }
          if (this->equivalentWithMemo(i, j)) {
            // Check the consistency
            for (const auto action: this->alphabet) {
              if (!this->equivalentWithMemo(this->discreteSuccessors.at({i, action}),
                                            this->discreteSuccessors.at({j, action}))) {
                BOOST_LOG_TRIVIAL(debug) << "Observation table is inconsistent because of the discrete successors of "
                                         << this->prefixes.at(i) << " and " << this->prefixes.at(j)
                                         << " with action " << action;
                resolveDiscreteInconsistency(i, j, action);
                return false;
              }
            }
            if (!this->equivalentWithMemo(this->continuousSuccessors.at(i), this->continuousSuccessors.at(j))) {
              // The observation table is inconsistent due to a continuous successor
              BOOST_LOG_TRIVIAL(debug) << "Observation table is inconsistent because of the continuous successors of "
                                       << this->prefixes.at(i) << " and " << this->prefixes.at(j);
              resolveContinuousInconsistency(i, j);

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
        // Skip if p is not a boundary
        if (this->inP(successorIndex)) {
          continue;
        }
        // Skip if p has equality constraints in \f$\mathbb{T}_{i,N}\f$.
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
        BOOST_LOG_TRIVIAL(debug) << "Adding a prefix " << prefix << " to P";
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
     * @todo This function is too complex. We need a refactoring.
     */
    TimedAutomaton generateHypothesis() {
      // map from the index in this->prefix to the corresponding state
      std::unordered_map<std::size_t, std::shared_ptr<TAState>> indexToState;
      std::unordered_map<std::shared_ptr<TAState>, std::vector<std::size_t>> stateToIndices;
      std::vector<std::shared_ptr<TAState>> states;
      // We have a temporary vector of prefixes because we can modify it to handle the exteriors
      auto tmpPrefixes = prefixes;
      // TODO: Write what this is for
      auto refreshTmpPrefixes = [&](std::size_t root) {
        std::queue<std::size_t> currentQueue;
        currentQueue.push(root);
        while (!currentQueue.empty()) {
          const auto currentIndex = currentQueue.front();
          currentQueue.pop();
          if (this->continuousSuccessors.find(currentIndex) != this->continuousSuccessors.end()) {
            const auto continuousIndex = this->continuousSuccessors.at(currentIndex);
            tmpPrefixes.at(continuousIndex) = tmpPrefixes.at(currentIndex).successor();
            currentQueue.push(continuousIndex);
          }
          for (const auto a: this->alphabet) {
            if (this->discreteSuccessors.find({currentIndex, a}) != this->discreteSuccessors.end()) {
              const auto discreteIndex = this->discreteSuccessors.at({currentIndex, a});
              tmpPrefixes.at(discreteIndex) = tmpPrefixes.at(currentIndex).successor(a);
              currentQueue.push(discreteIndex);
            }
          }
        }
      };

      /*!
       * @brief Make a state corresponding to the given index
       *
       * @pre indexToState[index] should not be set
       * @post states, indexToState and stateToIndex are appropriately updated
       */
      auto addState = [&](std::size_t index) {
        auto state = std::make_shared<TAState>(this->isMatch(index));
        indexToState[index] = state;
        auto it = stateToIndices.find(state);
        if (it == stateToIndices.end()) {
          stateToIndices[state] = {index};
        } else {
          // This should not happen.
          BOOST_LOG_TRIVIAL(error)
            << "Something wrong happened";
          abort();
          it->second.push_back(index);
        }
        states.push_back(state);

        return state;
      };

      /*!
       * @brief Merge continuous successors to the state corresponding to the given index
       *
       * @pre There is a state corresponding to initialSourceIndex
       * @post All the continuous successors corresponds to the same state as initialSourceIndex
       */
      const auto mergeContinuousSuccessors = [&](const std::size_t initialSourceIndex) {
        auto sourceIndex = initialSourceIndex;
        const auto state = indexToState[initialSourceIndex];
        auto nextIndex = this->continuousSuccessors[sourceIndex];
        // Include all the continuous successors to the state
        while (this->inP(nextIndex)) {
          if (isMatch(sourceIndex) == isMatch(nextIndex)) {
            indexToState[nextIndex] = state;
            stateToIndices[state].push_back(nextIndex);
          } else {
            BOOST_LOG_TRIVIAL(error)
              << "We have not implemented such a case that an unobservable transition is necessary";
            abort();
          }
          sourceIndex = nextIndex;
          nextIndex = this->continuousSuccessors[nextIndex];
        }

        // Our optimization to merge the continuous exterior
        if (isMatch(sourceIndex) == isMatch(nextIndex)) {
          indexToState[nextIndex] = state;
          stateToIndices[state].push_back(nextIndex);
          // TODO: Understand what the following does
          tmpPrefixes.at(sourceIndex).removeEqualityUpperBoundAssign();
          refreshTmpPrefixes(sourceIndex);
        } else {
          BOOST_LOG_TRIVIAL(error)
            << "We have not implemented such a case that an unobservable transition is necessary";
          abort();
        }
      };

      auto initialState = addState(0);
      // construct the initial state
      mergeContinuousSuccessors(0);
      // vector of states and actions such that the discrete successor is not in P
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
          // This is used when making guards
          std::unordered_map<std::shared_ptr<TAState>, TimedCondition> sourceMap;
          for (auto it = newStateIndices.begin(); it != newStateIndices.end(); ++it) {
            // Skip if there is no discrete successors in the observation table
            if (this->discreteSuccessors.find(std::make_pair(*it, action)) == this->discreteSuccessors.end()) {
              continue;
            }
            // q in the following diagram
            const auto discreteAfterContinuous = this->discreteSuccessors.at(std::make_pair(*it, action));
            // q' in the following diagram
            const auto discrete = this->discreteSuccessors.at(std::make_pair(*std::prev(it), action));
            // Add states only if the successor is also in P
            if (!this->inP(discreteAfterContinuous)) {
              discreteBoundaries.emplace_back(*it, action);
              continue;
            }
            /*
             * Check if the following q and q'' are similar. If yes, we merge q with q'
             *          *std::prev(it) ---cont--> *it
             *           |                        |
             *         action                   action
             *           |                        |
             *           v                        v
             *           q'--------cont--> q'' ~~ q
             */
            // we just add a new state if std::prev(it) does not exist or q' in the above diagram is not in P
            if (it == newStateIndices.begin() || !this->inP(discrete)) {
              auto successor = addState(discreteAfterContinuous);
              newStates.push(successor);
              mergeContinuousSuccessors(discreteAfterContinuous);
              sourceMap[successor] = tmpPrefixes.at(*it).getTimedCondition();
            } else {
              // q'' in the above diagram
              const auto continuousAfterDiscrete = this->continuousSuccessors[discrete];

              if (this->equivalentWithMemo(continuousAfterDiscrete, discreteAfterContinuous)) {
                // merge q and q'' in the above diagram
                indexToState[discreteAfterContinuous] = indexToState.at(discrete);
                stateToIndices[indexToState.at(discrete)].push_back(discreteAfterContinuous);

                // We use the convexity of the timed conditions of this and its continuous successor
                if (sourceMap[indexToState.at(discrete)].size() == tmpPrefixes.at(discreteAfterContinuous).getTimedCondition().size()) {
                  sourceMap[indexToState.at(discrete)] = sourceMap[indexToState.at(discrete)].convexHull(
                          tmpPrefixes.at(discreteAfterContinuous).getTimedCondition());
                }
              } else {
                // Otherwise, we create a new state
                const auto successor = addState(discreteAfterContinuous);
                newStates.push(successor);
                sourceMap[successor] = tmpPrefixes.at(*it).getTimedCondition();
              }

              mergeContinuousSuccessors(discreteAfterContinuous);
            }
          }

          if (!sourceMap.empty()) {
            newState->next[action].reserve(sourceMap.size());
            for (const auto&[target, timedCondition]: sourceMap) {
              newState->next[action].emplace_back(target.get(),
                                                  std::vector<std::pair<ClockVariables, std::optional<ClockVariables>>>{
                                                          std::make_pair(timedCondition.size(), std::nullopt)},
                                                  timedCondition.toGuard());
            }
          }
        }
      }

      //! Construct transitions by discrete immediate exteriors
      std::sort(discreteBoundaries.begin(), discreteBoundaries.end());
      discreteBoundaries.erase(std::unique(discreteBoundaries.begin(), discreteBoundaries.end()),
                               discreteBoundaries.end());
      for (auto[sourceIndex, action]: discreteBoundaries) {
        const auto originalSourceIndex = sourceIndex;

        ExternalTransitionMaker transitionMaker;
        auto targetIndex = this->discreteSuccessors.at(std::make_pair(sourceIndex, action));
        unsigned long jumpedTargetIndex;
        RenamingRelation renamingRelation;

        bool skipped = false;
        {
          if (this->pIndices.find(targetIndex) == this->pIndices.end()) {
            // Find a successor in P
            auto it = std::find_if(this->closedRelation.at(targetIndex).begin(),
                                   this->closedRelation.at(targetIndex).end(), [&](const auto &rel) {
                      return this->inP(rel.first);
                    });
            jumpedTargetIndex = it->first;
            renamingRelation = it->second;
          } else {
            jumpedTargetIndex = targetIndex;
          }
          auto jumpedState = indexToState.at(jumpedTargetIndex);
          transitionMaker.add(indexToState.at(jumpedTargetIndex), renamingRelation,
                              tmpPrefixes.at(sourceIndex).getTimedCondition(),
                              tmpPrefixes.at(jumpedTargetIndex).getTimedCondition());
          indexToState[targetIndex] = jumpedState;
          stateToIndices.at(jumpedState).push_back(targetIndex);
        }

        while (this->continuousSuccessors.find(sourceIndex) != this->continuousSuccessors.end()) {
          sourceIndex = this->continuousSuccessors.at(sourceIndex);
          BOOST_LOG_TRIVIAL(trace) << "Constructing a transition from " << tmpPrefixes.at(sourceIndex);
          {
            auto it = this->discreteSuccessors.find(std::make_pair(sourceIndex, action));
            if (it == this->discreteSuccessors.end()) {
              // Include immediate exterior
              if (!skipped) {
                transitionMaker.includeImmediateExterior(indexToState.at(jumpedTargetIndex), renamingRelation);
              }
              continue;
            }
            targetIndex = it->second;
            // We do not add new state if the state is already constructed
            if (indexToState.find(targetIndex) != indexToState.end() ||
                this->pIndices.find(targetIndex) != this->pIndices.end()) {
              skipped = true;
              continue;
            } else {
              skipped = false;
            }
          }
          // Find a successor in P
          auto it = std::find_if(this->closedRelation.at(targetIndex).begin(),
                                 this->closedRelation.at(targetIndex).end(), [&](const auto &rel) {
                    return this->inP(rel.first);
                  });
          // There is a jumped target index because the observation table is closed.
          assert(it != this->closedRelation.at(targetIndex).end());
          jumpedTargetIndex = it->first;
          assert(this->inP(jumpedTargetIndex));
          renamingRelation = it->second;
          auto jumpedState = indexToState.at(jumpedTargetIndex);

          transitionMaker.add(jumpedState, renamingRelation,
                              tmpPrefixes.at(sourceIndex).getTimedCondition(),
                              tmpPrefixes.at(jumpedTargetIndex).getTimedCondition(),
                              tmpPrefixes.at(sourceIndex).immediatePrefix()->getTimedCondition());
          indexToState[targetIndex] = jumpedState;
          stateToIndices.at(jumpedState).push_back(targetIndex);
        }
        auto newTransitions = transitionMaker.make();
        if (!newTransitions.empty()) {
          indexToState[originalSourceIndex]->next[action].insert(indexToState[originalSourceIndex]->next[action].end(),
                                                                 newTransitions.begin(), newTransitions.end());
        }
      }

      //! @todo We need to implement transitions by continuous immediate exteriors to support unobservable transitions

      // Assert the totality of the constructed DTA
      assert(std::all_of(this->pIndices.begin(), this->pIndices.end(), [&](std::size_t pIndex) {
        return indexToState.find(pIndex) != indexToState.end();
      }));
      assert(std::all_of(this->discreteSuccessors.begin(), this->discreteSuccessors.end(), [&](const auto &pair) {
        auto sourceStateIndex = pair.first.first;
        auto action = pair.first.second;
        auto targetStateIndex = pair.second;
        auto targetState = indexToState.at(targetStateIndex);
        return indexToState.at(sourceStateIndex)->next[action].end() !=
               std::find_if(indexToState.at(sourceStateIndex)->next[action].begin(),
                            indexToState.at(sourceStateIndex)->next[action].end(),
                            [&](const TATransition &transition) {
                              return transition.target == targetState.get();
                            });
      }));
      for (std::size_t index = 0; index < this->prefixes.size(); index++) {
        if (indexToState.end() == indexToState.find(index)) {
          BOOST_LOG_TRIVIAL(warning) << "Partial transitions is detected";
          // abort();
        }
      }

      return TimedAutomaton{{states, {initialState}}, TimedAutomaton::makeMaxConstants(states)}.simplify();
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

      stream << "Number of membership queries: " << this->memOracle->count() << "\n";

      return stream;
    }
  };
}

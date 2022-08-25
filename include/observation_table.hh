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
      leftRow.reserve(leftRow.size() + newSuffixes.size());
      auto rightRow = this->table.at(j);
      rightRow.reserve(rightRow.size() + newSuffixes.size());
      auto tmpSuffixes = this->suffixes;
      tmpSuffixes.reserve(tmpSuffixes.size() + newSuffixes.size());
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

    /*!
     * @brief Returns if prefixes[i] has a discrete successor in the observation table
     */
    [[nodiscard]] bool hasDiscreteSuccessor(std::size_t i, Alphabet c) const {
      return this->discreteSuccessors.find(std::make_pair(i, c)) != this->discreteSuccessors.end();
    }

    /*!
     * @brief Returns if prefixes[i] has a continuous successor in the observation table
     */
    [[nodiscard]] bool hasContinuousSuccessor(std::size_t i) const {
      return this->continuousSuccessors.find(i) != this->continuousSuccessors.end();
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
          auto previousNewSuffixes = allPredecessors;
          do {
            decltype(allPredecessors) newSuffixes;
            for (const auto &suffix: previousNewSuffixes) {
              try {
                newSuffixes.push_back(suffix.predecessor());
                BOOST_LOG_TRIVIAL(info) << suffix.predecessor();
              } catch (...) {
              }
            }
            if (newSuffixes.empty()) {
              BOOST_LOG_TRIVIAL(fatal) << "Something is wrong in resolving continuous inconsistency";
              abort();
            }
            previousNewSuffixes = std::move(newSuffixes);
            std::copy(previousNewSuffixes.begin(), previousNewSuffixes.end(), std::back_inserter(allPredecessors));
          } while (equivalent(i, j, allPredecessors));
          BOOST_LOG_TRIVIAL(info) << "Found longer predecessors.";
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
     * @brief The class to manage the correspondence between states and indices.
     */
    class StateManager {
      // map from the index in this->prefix to the corresponding state
      std::unordered_map<std::size_t, std::shared_ptr<TAState>> indexToState;
      std::unordered_map<std::shared_ptr<TAState>, std::vector<std::size_t>> stateToIndices;
    public:
      [[nodiscard]] std::shared_ptr<TAState> toState(const std::size_t index) const {
        return indexToState.at(index);
      }

      [[nodiscard]] std::vector<std::size_t> toIndices(const std::shared_ptr<TAState>& state) const {
        return stateToIndices.at(state);
      }

      [[nodiscard]] bool isNew(const std::shared_ptr<TAState>& state) const {
        auto it = stateToIndices.find(state);
        return it == stateToIndices.end();
      }

      [[nodiscard]] bool isNew(std::size_t index) const {
        auto it = indexToState.find(index);
        return it == indexToState.end();
      }

      void add(const std::shared_ptr<TAState>& state, const std::size_t index) {
        indexToState[index] = state;
        auto it = stateToIndices.find(state);
        if (it == stateToIndices.end()) {
          stateToIndices[state] = {index};
        } else {
          it->second.push_back(index);
        }
      }
    };

    /*!
     * @brief Construct a hypothesis DTA from the current timed observation table
     *
     * @pre The observation table is closed, consistent, and exterior-consistent.
     * @todo We currently construct only the DTAs without unobservable transitions.
     */
    TimedAutomaton generateHypothesis() {
      StateManager stateManager;
      std::vector<std::shared_ptr<TAState>> states;
      // We have a temporary vector of prefixes because we can modify it when merging states
      auto tmpPrefixes = prefixes;
      /*!
       * @berief Propagate the update in tmpPrefixes to the successors
       *
       * @param[in] root The root of the modification. The updated row index should be given.
       * This function should be called after updating tmpPrefixes.
       */
      auto refreshTmpPrefixes = [&](std::size_t root) {
        std::queue<std::size_t> currentQueue;
        currentQueue.push(root);
        while (!currentQueue.empty()) {
          const auto currentIndex = currentQueue.front();
          currentQueue.pop();
          if (this->hasContinuousSuccessor(currentIndex)) {
            const auto continuousIndex = this->continuousSuccessors.at(currentIndex);
            tmpPrefixes.at(continuousIndex) = tmpPrefixes.at(currentIndex).successor();
            currentQueue.push(continuousIndex);
          }
          for (const auto a: this->alphabet) {
            if (this->hasDiscreteSuccessor(currentIndex, a)) {
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
        assert(stateManager.isNew(state));
        stateManager.add(state, index);
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
        const auto state = stateManager.toState(initialSourceIndex);
        auto nextIndex = this->continuousSuccessors.at(sourceIndex);
        // Include all the continuous successors to the state
        while (this->inP(nextIndex)) {
          if (isMatch(sourceIndex) == isMatch(nextIndex)) {
            stateManager.add(state, nextIndex);
          } else {
            BOOST_LOG_TRIVIAL(error)
              << "We have not implemented such a case that an unobservable transition is necessary";
            abort();
          }
          sourceIndex = nextIndex;
          nextIndex = this->continuousSuccessors.at(nextIndex);
        }

        // Our optimization to merge the continuous exterior
        if (isMatch(sourceIndex) == isMatch(nextIndex)) {
          stateManager.add(state, nextIndex);
          // Include the exterior
          tmpPrefixes.at(sourceIndex).removeEqualityUpperBoundAssign();
          refreshTmpPrefixes(sourceIndex);
        } else {
          BOOST_LOG_TRIVIAL(error)
            << "We have not implemented such a case that an unobservable transition is necessary";
          abort();
        }
      };

      const auto initialState = addState(0);
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
        const auto newStateIndices = stateManager.toIndices(newState);
        for (const auto action: alphabet) {
          // TargetState -> the timed condition to reach that state
          // This is used when making guards
          std::unordered_map < std::shared_ptr<TAState>, TimedCondition > sourceMap;
          for (const auto &newStateIndex: newStateIndices) {
            // Skip if there is no discrete nor continuous successors in the observation table
            if (!this->hasDiscreteSuccessor(newStateIndex, action)) {
              continue;
            }
            // q' in the following diagram
            const auto discrete = this->discreteSuccessors.at(std::make_pair(newStateIndex, action));
            // Add states only if the successor is also in P
            if (!this->inP(discrete)) {
              discreteBoundaries.emplace_back(newStateIndex, action);
            } else {
              const auto successor = addState(discrete);
              newStates.push(successor);
              if (this->hasContinuousSuccessor(discrete)) {
                mergeContinuousSuccessors(discrete);
              }
              sourceMap[successor] = tmpPrefixes.at(newStateIndex).getTimedCondition();
            }
            if (this->hasContinuousSuccessor(newStateIndex)) {
              // Try to merge q'' and q_a in the following diagram
              // q in the following diagram
              const auto continuous = this->continuousSuccessors.at(newStateIndex);
              if (!this->hasDiscreteSuccessor(continuous, action) || !this->hasContinuousSuccessor(discrete)) {
                continue;
              }
              // q_a in the following diagram
              const auto discreteAfterContinuous = this->discreteSuccessors.at(std::make_pair(continuous, action));
              // q'' in the following diagram
              const auto continuousAfterDiscrete = this->continuousSuccessors.at(discrete);
              /*
               * Check if the following q_a and q'' are similar. If yes, we merge q_a with q'
               *      newStateIndex--cont-------->  q
               *           |                        |
               *         action                   action
               *           |                        |
               *           v                        v
               *           q'--------cont--> q'' ~~ q_a
               */

              if (this->equivalentWithMemo(continuousAfterDiscrete, discreteAfterContinuous)) {
                // merge q_a and q'' in the above diagram
                stateManager.add(stateManager.toState(continuousAfterDiscrete), discreteAfterContinuous);

                // We use the convexity of the timed conditions of this and its continuous successor
                if (sourceMap[stateManager.toState(discrete)].size() ==
                    tmpPrefixes.at(discreteAfterContinuous).getTimedCondition().size()) {
                  sourceMap[stateManager.toState(discrete)] = sourceMap[stateManager.toState(discrete)].convexHull(
                          tmpPrefixes.at(discreteAfterContinuous).getTimedCondition());
                }
              } else if (stateManager.isNew(discreteAfterContinuous)) {
                // Otherwise, we create a new state
                const auto successor = addState(discreteAfterContinuous);
                newStates.push(successor);
                mergeContinuousSuccessors(discreteAfterContinuous);
                sourceMap[successor] = tmpPrefixes.at(newStateIndex).getTimedCondition();
              }
            }
          }

          // Make transitions
          if (!sourceMap.empty()) {
            newState->next[action].reserve(sourceMap.size());
            for (const auto &[target, timedCondition]: sourceMap) {
              newState->next[action].emplace_back(target.get(), timedCondition.size(), timedCondition.toGuard());
            }
          }
        }
      }

      //! Construct transitions by discrete immediate exteriors
      std::sort(discreteBoundaries.begin(), discreteBoundaries.end());
      discreteBoundaries.erase(std::unique(discreteBoundaries.begin(), discreteBoundaries.end()),
                               discreteBoundaries.end());
      for (const auto &[sourceIndex, action]: discreteBoundaries) {
        ExternalTransitionMaker transitionMaker;
        const auto addNewTransition = [&](std::size_t source, std::size_t jumpedTarget, std::size_t target,
                                          const auto &renamingRelation) {
          auto jumpedState = stateManager.toState(jumpedTarget);
          transitionMaker.add(jumpedState, renamingRelation,
                              tmpPrefixes.at(source).getTimedCondition(),
                              tmpPrefixes.at(jumpedTarget).getTimedCondition());
          if (stateManager.isNew(target)) {
            stateManager.add(jumpedState, target);
          }
        };
        // The target state of the transitions, which should be in ext(P)
        const auto targetIndex = this->discreteSuccessors.at(std::make_pair(sourceIndex, action));
        assert(!this->inP(targetIndex));
        // Find a successor in P
        auto it = std::find_if(this->closedRelation.at(targetIndex).begin(),
                               this->closedRelation.at(targetIndex).end(), [&](const auto &rel) {
                  return this->inP(rel.first);
                });
        // The target state of the transitions after mapping to P.
        const auto jumpedTargetIndex= it->first;
        // The renaming relation connecting targetIndex and jumpedTargetIndex
        const RenamingRelation renamingRelation = it->second;

        addNewTransition(sourceIndex, jumpedTargetIndex, targetIndex, renamingRelation);

        auto newTransitions = transitionMaker.make();
        if (!newTransitions.empty()) {
          stateManager.toState(sourceIndex)->next[action].reserve(
                  stateManager.toState(sourceIndex)->next[action].size() + newTransitions.size());
          std::move(newTransitions.begin(), newTransitions.end(),
                    std::back_inserter(stateManager.toState(sourceIndex)->next[action]));
        }
      }

      //! @todo We need to implement transitions by continuous immediate exteriors to support unobservable transitions

      // Assert the totality of the constructed DTA
      assert(std::all_of(this->pIndices.begin(), this->pIndices.end(), [&](std::size_t pIndex) {
        return !stateManager.isNew(pIndex);
      }));
      assert(std::all_of(this->discreteSuccessors.begin(), this->discreteSuccessors.end(), [&](const auto &pair) {
        auto sourceStateIndex = pair.first.first;
        auto action = pair.first.second;
        auto targetStateIndex = pair.second;
        auto targetState = stateManager.toState(targetStateIndex);
        return stateManager.toState(sourceStateIndex)->next[action].end() !=
               std::find_if(stateManager.toState(sourceStateIndex)->next[action].begin(),
                            stateManager.toState(sourceStateIndex)->next[action].end(),
                            [&](const TATransition &transition) {
                              return transition.target == targetState.get();
                            });
      }));
      for (std::size_t index = 0; index < this->prefixes.size(); index++) {
        if (stateManager.isNew(index)) {
          BOOST_LOG_TRIVIAL(warning) << "Partial transitions is detected";
          abort();
        }
      }

      return TimedAutomaton{{states, {initialState}}, TimedAutomaton::makeMaxConstants(states)}.simplify();
    }

    std::ostream

    &
    printDetail(std::ostream
                &stream) const {
      printStatistics(stream);
      stream << "P is as follows\n";
      for (
        const auto &pIndex
              : this->pIndices) {
        stream << this->prefixes.
                at(pIndex)
               << "\n";
      }
      stream << "S is as follows\n";
      for (
        const auto &suffix
              : this->suffixes) {
        stream << suffix << "\n";
      }

      return
              stream;
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

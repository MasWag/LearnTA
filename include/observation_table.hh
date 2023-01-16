/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include <utility>
#include <vector>
#include <stack>
#include <list>
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
#include "internal_transition_maker.hh"
#include "external_transition_maker.hh"
#include "counterexample_analyzer.hh"
#include "neighbor_conditions.hh"
#include "imprecise_clock_handler.hh"

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
    // Maybe we should use vector instead
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
      for (std::size_t prefixIndex = 0; prefixIndex < prefixes.size(); ++prefixIndex) {
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
      // We add continuous successors to P if it is clearly not saturated
      if (!equivalentWithMemo(continuousSuccessors.at(index), index)) {
        this->moveToP(continuousSuccessors.at(index));
      }
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

    boost::unordered_map<std::tuple<std::size_t, std::size_t, BackwardRegionalElementaryLanguage>, std::pair<std::size_t, bool>> equivalentWithColumnCache;

    /*!
     * @brief Check if the given rows remain equivalent with a new suffix
     */
    [[nodiscard]] bool equivalent(std::size_t i, std::size_t j, const BackwardRegionalElementaryLanguage &newSuffix) {
      // First, we use the cache
      auto key = std::make_tuple(i, j, newSuffix);
      {
        auto it = equivalentWithColumnCache.find(key);
        if (it != equivalentWithColumnCache.end() && this->suffixes.size() == it->second.first) {
          return it->second.second;
        }
      }
      // Then, we try the known renaming relations
      {
        auto equivalentPrefixesIT = this->closedRelation.find(i);
        if (equivalentPrefixesIT != this->closedRelation.end()) {
          auto it = equivalentPrefixesIT->second.find(j);
          if (it != this->closedRelation.at(i).end()) {
            if (equivalent(i, j, newSuffix, it->second)) {
              equivalentWithColumnCache[key] = std::make_pair(this->suffixes.size(), true);
              return true;
            }
          }
        }
      }
      // Finally, we try to find an equivalent renaming
      auto leftRow = this->table.at(i);
      leftRow.push_back(this->memOracle->query(prefixes.at(i) + newSuffix));
      auto rightRow = this->table.at(j);
      rightRow.push_back(this->memOracle->query(prefixes.at(j) + newSuffix));
      auto newSuffixes = this->suffixes;
      newSuffixes.push_back(newSuffix);
      const auto result = findEquivalentRenaming(this->prefixes.at(i), leftRow, this->prefixes.at(j), rightRow,
                                                 newSuffixes).has_value();
      equivalentWithColumnCache[key] = std::make_pair(this->suffixes.size(), result);

      return result;
    }

    /*!
     * @brief Check if the given renaming relation remains an evidence of the equivalence with a new suffix
     */
    [[nodiscard]] bool equivalent(std::size_t i, std::size_t j, const BackwardRegionalElementaryLanguage &newSuffix,
                                  const RenamingRelation &renaming) const {
      const auto leftPrefix = this->prefixes.at(i);
      auto leftRow = this->table.at(i);
      leftRow.push_back(this->memOracle->query(prefixes.at(i) + newSuffix));
      const auto rightPrefix = this->prefixes.at(j);
      auto rightRow = this->table.at(j);
      rightRow.push_back(this->memOracle->query(prefixes.at(j) + newSuffix));
      auto newSuffixes = this->suffixes;
      newSuffixes.push_back(newSuffix);

      return equivalence(leftPrefix, leftRow, rightPrefix, rightRow, newSuffixes, renaming);
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

    /*!
     * @brief Split the states based on the imprecise clocks if necessary
     *
     * @param [inout] originalStates The states
     * @param [in] needSplit The states need to be split
     */
    void splitStates(std::vector<std::shared_ptr<TAState>> &originalStates,
                     const std::shared_ptr<TAState> &initialState,
                     const std::vector<TAState *> &needSplit) const;

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
        if (this->inP(i)) {
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
                if (this->inP(targetIt->first)) {
                  found = true;
                  break;
                } else {
                  ++targetIt;
                }
              } else {
                targetIt = it->second.erase(targetIt);
              }
            }
          }
        }

        // We have no idea of the target prefix, and we find an equivalent row
        if (!found) {
          // First, we try to "jump" to the same state
          found = std::any_of(this->pIndices.begin(), this->pIndices.end(), [&](const auto j) {
            return this->continuousSuccessors.at(j) == i && equivalentWithMemo(i, j);
          });
          if (!found) {
            found = std::any_of(this->pIndices.begin(), this->pIndices.end(), [&](const auto j) {
              return equivalentWithMemo(i, j);
            });
          }
        }
        if (!found) {
          // The observation table is not closed
#ifndef NDEBUG
          const auto prePSize = this->pIndices.size();
#endif
          this->moveToP(i);
          assert(prePSize < this->pIndices.size());
          BOOST_LOG_TRIVIAL(debug) << "Observation table is not closed because of " << this->prefixes.at(i);
          this->refreshTable();
          return false;
        }
      }

      // Assert that we have prefixes for any successors
      assert(std::all_of(this->pIndices.begin(), this->pIndices.end(), [&](const auto &pIndex) {
        const auto successor = this->continuousSuccessors.at(pIndex);
        return successor < this->prefixes.size();
      }));
      assert(std::all_of(this->pIndices.begin(), this->pIndices.end(), [&](const auto &pIndex) {
        return std::all_of(this->alphabet.begin(), this->alphabet.end(), [&](const auto &action) {
          const auto successor = this->discreteSuccessors.at(std::make_pair(pIndex, action));
          return successor < this->prefixes.size();
        });
      }));
      // Assert that we have the information of the closedness in closed relation
      for (std::size_t pIndex = 0; pIndex < this->prefixes.size(); ++pIndex) {
        assert(this->inP(pIndex) || std::any_of(this->closedRelation.at(pIndex).begin(),
                                                this->closedRelation.at(pIndex).end(),
                                                [&](const auto &rPair) {
                                                  return this->inP(rPair.first);
                                                }));
      }
      assert(std::all_of(this->pIndices.begin(), this->pIndices.end(), [&](const auto &pIndex) {
        return this->pIndices.find(this->continuousSuccessors.at(pIndex)) != this->pIndices.end() ||
               std::any_of(this->closedRelation.at(this->continuousSuccessors.at(pIndex)).begin(),
                           this->closedRelation.at(this->continuousSuccessors.at(pIndex)).end(),
                           [&](const auto &rPair) {
                             return this->inP(rPair.first);
                           });
      }));
      assert(std::all_of(this->pIndices.begin(), this->pIndices.end(), [&](const auto &pIndex) {
        return std::all_of(this->alphabet.begin(), this->alphabet.end(), [&](const auto &action) {
          const auto successor = this->discreteSuccessors.at(std::make_pair(pIndex, action));
          return this->inP(successor) || std::any_of(this->closedRelation.at(successor).begin(),
                                                     this->closedRelation.at(successor).end(),
                                                     [&](const std::pair<std::size_t, RenamingRelation> &rPair) {
                                                       return this->inP(rPair.first);
                                                     });
        });
      }));
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
     * @brief Try to resolve an inconsistency due to continuous successors
     *
     * @pre equivalent(i, j)
     * @pre !equivalent(continuousSuccessors(i), continuousSuccessors(j))
     * @post !equivalent(i, j)
     *
     * @return If the inconsistency is resolved
     */
    [[nodiscard]] bool resolveContinuousInconsistency(std::size_t i, std::size_t j) {
      // Find a single witness of the inconsistency
      auto it = std::find_if_not(suffixes.begin(), suffixes.end(), [&](const auto &suffix) {
        return equivalent(i, j, suffix.predecessor());
      });
      // We may fail to resolve continuous inconsistency
      if (it == suffixes.end()) {
        return false;
      }
      const auto newSuffix = it->predecessor();
      BOOST_LOG_TRIVIAL(debug) << "New suffix " << newSuffix << " is added";
      suffixes.push_back(newSuffix);

      this->refreshTable();
      // i and j should not be equivalent after adding the new suffix
      assert(!this->equivalentWithMemo(i, j));

      return true;
    }

    /*!
     * @brief Handle inactive clock variables in the DTA construction
     *
     * A clock variable is deactivated if its value may not be the same as the one in the corresponding reduction in the recognizable timed language
     * This happens when we use (u, Λ, u', Λ', R) such that (w, w') ∈ (u, Λ, u', Λ', R) ∧ (w, w'') ∈ (u, Λ, u', Λ', R) does not imply w' = w''
     * By definition of recognizable timed languages, such w, w', and w'' are equivalent with respect to any extension
     */
    static void handleInactiveClocks(std::vector<std::shared_ptr<TAState>> &states);

  public:
    /*!
     * @brief Construct a recognizable timed language corresponding to the observation table
     * @pre The observation table is closed, consistent, and exterior-consistent.
     */
    [[nodiscard]] RecognizableLanguage toRecognizable() {
      std::vector<ElementaryLanguage> P;
      std::vector<ElementaryLanguage> final;
      for (std::size_t i = 0; i < this->prefixes.size(); ++i) {
        if (this->inP(i)) {
          P.push_back(this->prefixes.at(i));
          if (this->isMatch(i)) {
            final.push_back(this->prefixes.at(i));
          }
        }
      }

      std::vector<SingleMorphism> morphisms;
      morphisms.reserve(this->closedRelation.size());
      for (auto &[i, mapping]: this->closedRelation) {
        if (!this->inP(i) && !mapping.empty()) {
          for (auto it = mapping.begin(); it != mapping.end();) {
            if (this->inP(it->first) && this->equivalentWithMemo(i, it->first)) {
              morphisms.emplace_back(this->prefixes.at(i),
                                     this->prefixes.at(it->first),
                                     it->second);
              break;
            } else {
              it = mapping.erase(it);
            }
          }
        }
      }

      return RecognizableLanguage{P, final, morphisms};
    }

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
            if (!this->equivalentWithMemo(this->continuousSuccessors.at(i), this->continuousSuccessors.at(j))
                && resolveContinuousInconsistency(i, j)) {
              BOOST_LOG_TRIVIAL(debug) << "Observation table is inconsistent because of the continuous successors of "
                                       << this->prefixes.at(i) << " and " << this->prefixes.at(j);
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
     * @brief Make the observation table exterior-saturated
     *
     * Observation table is exterior-saturated if for any \f$p \in P\f$, if \f$\mathrm{ext}^t(p) \not\in P\f$,
     * we have \f$\mathrm{ext}^t(p) \sim^{S, \top} p\f$.
     *
     * @returns If the observation table is already exterior-saturated
     */
    bool exteriorSaturate() {
      std::vector<std::size_t> newP;
      newP.reserve(pIndices.size());
      for (const std::size_t pIndex: pIndices) {
        const auto successorIndex = continuousSuccessors.at(pIndex);
        // Skip if p is not a boundary
        if (this->inP(successorIndex)) {
          continue;
        }
        if (equivalentWithMemo(successorIndex, pIndex)) {
          auto it = this->closedRelation.find(successorIndex);
          assert(it != this->closedRelation.end());
          auto it2 = it->second.find(pIndex);
          assert(it2 != it->second.end());
          if (it2->second.empty()) {
            continue;
          }
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

    bool renameConsistent() {
      for (auto &[i, mapping]: this->closedRelation) {
        if (!this->inP(i) && !mapping.empty()) {
          for (auto it = mapping.begin(); it != mapping.end();) {
            if (this->inP(it->first) && this->equivalentWithMemo(i, it->first)) {
              // find a variable that gets imprecise after morphism
              const auto &[rootIndex, rootRenaming] = *it;
              const auto rootLanguage = this->prefixes.at(rootIndex);
              const auto impreciseAfterMorphism = rootRenaming.impreciseClocks(rootLanguage.getTimedCondition());
              ++it;
              if (impreciseAfterMorphism.empty()) {
                continue;
              }
              // Confirm that such variables are not used in the renaming relation later
              std::stack<std::size_t> toSearch;
              toSearch.push(rootIndex);
              while(!toSearch.empty()) {
                const auto currentIndex = toSearch.top();
                toSearch.pop();
                if (this->inP(currentIndex)) {
                  // Add the successors
                  toSearch.push(this->continuousSuccessors.at(currentIndex));
                  for (const auto action: this->alphabet) {
                    toSearch.push(this->discreteSuccessors.at(std::make_pair(currentIndex, action)));
                  }
                } else {
                  const auto currentLanguage = this->prefixes.at(currentIndex);
                  const auto& currentCondition = currentLanguage.getTimedCondition();
                  for (const auto &[indexAfterMap, renaming]: this->closedRelation.at(currentIndex)) {
                    if (this->inP(indexAfterMap) && this->equivalentWithMemo(currentIndex, indexAfterMap)) {
                      // confirm that imprecise variables are not used
                      for (const ClockVariables clock: impreciseAfterMorphism) {
                        if (!currentCondition.isPoint(clock) &&
                            std::any_of(renaming.begin(), renaming.end(), is_second<std::size_t, std::size_t>(clock))) {
                          // clock is imprecise but used!!
                          BOOST_LOG_TRIVIAL(info) << "Observation table is renaming inconsistent: " << clock;
                          // Try to extend the suffixes
                          const TimedWord extension = currentLanguage.suffix(rootLanguage).sample();
                          for (const auto &suffix: this->suffixes) {
                            const auto newSuffix = BackwardRegionalElementaryLanguage::fromTimedWord(extension + suffix.sample());
                            if (!equivalent(i, rootIndex, newSuffix, rootRenaming) ||
                                !equivalent(currentIndex, indexAfterMap, newSuffix, renaming)) {
                              BOOST_LOG_TRIVIAL(info) << "renameConsistent: New suffix " << newSuffix << " is added";
                              this->suffixes.push_back(newSuffix);
                              this->refreshTable();
                              return false;
                            }
                            for (const auto &simple: (currentLanguage + suffix).enumerate()) {
                              const auto newSuffix2 = ForwardRegionalElementaryLanguage::fromTimedWord(simple.sample()).suffix(rootLanguage);
                              if (!equivalent(i, rootIndex, newSuffix2, rootRenaming) ||
                                  !equivalent(currentIndex, indexAfterMap, newSuffix2, renaming)) {
                                BOOST_LOG_TRIVIAL(info) << "renameConsistent: New suffix " << newSuffix2
                                                         << " is added";
                                this->suffixes.push_back(newSuffix2);
                                this->refreshTable();
                                return false;
                              }
                            }
                          }
                          BOOST_LOG_TRIVIAL(info) << "Failed to resolve the renaming inconsistency!!";
                        }
                      }
                    }
                  }
                }
              }
            } else {
              it = mapping.erase(it);
            }
          }
        }
      }
      return true;
    }

    /*!
     * @brief Refine the suffixes by the given counterexample
     *
     */
    void handleCEX(const TimedWord &cex) {
      auto newSuffixOpt = analyzeCEX(cex, *this->memOracle, this->toRecognizable(), this->suffixes);
      if (newSuffixOpt) {
        BOOST_LOG_TRIVIAL(debug) << "New suffix " << *newSuffixOpt << " is added";
        auto newSuffix = BackwardRegionalElementaryLanguage::fromTimedWord(*newSuffixOpt);
        suffixes.push_back(std::move(newSuffix));
        this->refreshTable();
      } else {
        BOOST_LOG_TRIVIAL(debug) << "Failed to find a new suffix. We add prefixes to P";
        const auto newPrefixes = ForwardRegionalElementaryLanguage::fromTimedWord(cex).prefixes();
        bool updated = false;
        for (const auto &newPrefix: newPrefixes) {
          auto it = std::find(this->prefixes.begin(), this->prefixes.end(), newPrefix);
          assert(it != this->prefixes.end());
          if (this->inP(std::distance(this->prefixes.begin(), it))) {
            continue;
          } else {
            updated = true;
            this->moveToP(std::distance(this->prefixes.begin(), it));
            if (!this->close()) {
              // The observation table is refined
              return;
            }
          }
        }
        if (!updated) {
          BOOST_LOG_TRIVIAL(error) << "Learning has got stuck!!";
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

      [[nodiscard]] std::vector<std::size_t> toIndices(const std::shared_ptr<TAState> &state) const {
        return stateToIndices.at(state);
      }

      [[nodiscard]] bool isNew(const std::shared_ptr<TAState> &state) const {
        auto it = stateToIndices.find(state);
        return it == stateToIndices.end();
      }

      [[nodiscard]] bool isNew(std::size_t index) const {
        auto it = indexToState.find(index);
        return it == indexToState.end();
      }

      void add(const std::shared_ptr<TAState> &state, const std::size_t index) {
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
     * @note We currently construct only the DTAs without unobservable transitions.
     */
    TimedAutomaton generateHypothesis() {
      StateManager stateManager;
      std::vector<std::shared_ptr<TAState>> states;

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
          InternalTransitionMaker sourceMap;
          for (const auto &newStateIndex: newStateIndices) {
#ifdef DEBUG
            BOOST_LOG_TRIVIAL(trace) << "Start exploration of the discrete successor from the prefix "
                                     << this->prefixes.at(newStateIndex) << " with action " << action;
#endif

            // Skip if there is no discrete successor in the observation table
            if (!this->hasDiscreteSuccessor(newStateIndex, action)) {
#ifdef DEBUG
              BOOST_LOG_TRIVIAL(trace) << "No discrete successor";
#endif
              continue;
            }
            // q' in the following diagram
            const auto discrete = this->discreteSuccessors.at(std::make_pair(newStateIndex, action));
            // Add states only if the successor is also in P
            if (!this->inP(discrete)) {
#ifdef DEBUG
              BOOST_LOG_TRIVIAL(trace) << "The discrete successor is not in P";
#endif
              discreteBoundaries.emplace_back(newStateIndex, action);
            } else {
#ifdef DEBUG
              BOOST_LOG_TRIVIAL(trace) << "The discrete successor is in P";
#endif
              if (stateManager.isNew(discrete)) {
#ifdef DEBUG
                BOOST_LOG_TRIVIAL(trace) << "The discrete successor is new";
#endif
                const auto successor = addState(discrete);
                newStates.push(successor);
#ifdef DEBUG
                BOOST_LOG_TRIVIAL(trace) << "Generate discrete transitions from " << this->prefixes.at(newStateIndex)
                                         << " with action " << action;
                BOOST_LOG_TRIVIAL(trace) << "Source: " << stateManager.toState(newStateIndex);
                BOOST_LOG_TRIVIAL(trace) << "Guard: " << this->prefixes.at(newStateIndex).getTimedCondition().toGuard();
                BOOST_LOG_TRIVIAL(trace) << "Target: " << successor;
#endif
                sourceMap.add(successor, this->prefixes.at(newStateIndex).getTimedCondition());
#ifdef DEBUG
                BOOST_LOG_TRIVIAL(trace) << "The new state: " << successor.get();
#endif
              }
              if (this->hasContinuousSuccessor(discrete)) {
#ifdef DEBUG
                BOOST_LOG_TRIVIAL(trace) << "The discrete successor has continuous successors";
#endif
                mergeContinuousSuccessors(discrete);
              }
            }
          }

          // Make transitions
          if (!sourceMap.empty()) {
            auto newTransitions = sourceMap.make();
            newState->next[action].reserve(newState->next[action].size() + newTransitions.size());
            std::move(newTransitions.begin(), newTransitions.end(), std::back_inserter(newState->next[action]));
          }
        }
      }

      ImpreciseClockHandler impreciseNeighbors;
      //! Construct transitions by discrete immediate exteriors
      std::sort(discreteBoundaries.begin(), discreteBoundaries.end());
      discreteBoundaries.erase(std::unique(discreteBoundaries.begin(), discreteBoundaries.end()),
                               discreteBoundaries.end());
      for (const auto &[sourceIndex, action]: discreteBoundaries) {
#ifdef DEBUG
        BOOST_LOG_TRIVIAL(debug) << "Constructing a transition from: " << this->prefixes.at(sourceIndex)
                                 << " with action " << action;
#endif
        ExternalTransitionMaker transitionMaker;
        const auto addNewTransition = [&](std::size_t source, std::size_t jumpedTarget, std::size_t target,
                                          const auto &renamingRelation) {
          auto jumpedState = stateManager.toState(jumpedTarget);
          transitionMaker.add(jumpedState, renamingRelation,
                              this->prefixes.at(source).getTimedCondition(),
                              this->prefixes.at(jumpedTarget).getTimedCondition());
          impreciseNeighbors.push(jumpedState.get(), renamingRelation, this->prefixes.at(jumpedTarget));
          if (stateManager.isNew(target)) {
            stateManager.add(jumpedState, target);
          }
        };
        // The target state of the transitions, which should be in ext(P)
        const auto targetIndex = this->discreteSuccessors.at(std::make_pair(sourceIndex, action));
        if (!stateManager.isNew(targetIndex)) {
#ifdef DEBUG
          BOOST_LOG_TRIVIAL(trace) << "The boundary is already handled: " << this->prefixes.at(sourceIndex) << " "
                                   << action;
#endif
          continue;
        }
        assert(!this->inP(targetIndex));
        // Find a successor in P
        auto it = std::find_if(this->closedRelation.at(targetIndex).begin(),
                               this->closedRelation.at(targetIndex).end(), [&](const auto &rel) {
                  return this->inP(rel.first);
                });
        assert(it != this->closedRelation.at(targetIndex).end());
        // The target state of the transitions after mapping to P.
        const auto jumpedTargetIndex = it->first;
        // The renaming relation connecting targetIndex and jumpedTargetIndex
        RenamingRelation renamingRelation = it->second;
#ifdef DEBUG
        BOOST_LOG_TRIVIAL(debug) << "source: " << this->prefixes.at(sourceIndex);
        BOOST_LOG_TRIVIAL(debug) << "action: " << action;
        BOOST_LOG_TRIVIAL(debug) << "target: " << this->prefixes.at(jumpedTargetIndex);
        BOOST_LOG_TRIVIAL(debug) << "renaming: " << renamingRelation;
#endif

        // renamingRelation should not have the last variable on the left hand side.
        renamingRelation.eraseLeft(this->prefixes.at(sourceIndex).getTimedCondition().size());
        addNewTransition(sourceIndex, jumpedTargetIndex, targetIndex, renamingRelation);

        auto newTransitions = transitionMaker.make();
        if (!newTransitions.empty()) {
          stateManager.toState(sourceIndex)->next[action].reserve(
                  stateManager.toState(sourceIndex)->next[action].size() + newTransitions.size());
          std::move(newTransitions.begin(), newTransitions.end(),
                    std::back_inserter(stateManager.toState(sourceIndex)->next[action]));
        }
      }

      //! Construct transitions by continuous immediate exteriors
      for (const auto source: this->pIndices) {
        const auto continuousSuccessor = this->continuousSuccessors.at(source);
        if (this->inP(continuousSuccessor)) {
          continue;
        }
        const auto sourceState = stateManager.toState(continuousSuccessor);
        // Find a successor in P
        const auto it = std::find_if(this->closedRelation.at(continuousSuccessor).begin(),
                                     this->closedRelation.at(continuousSuccessor).end(), [&](const auto &rel) {
                  return this->inP(rel.first);
                });
        assert(it != this->closedRelation.at(continuousSuccessor).end());
        // The continuous successor after mapping to P.
        const auto jumpedSourceIndex = it->first;
        const auto jumpedSourceState = stateManager.toState(jumpedSourceIndex);
        const auto jumpedSourceCondition = this->prefixes.at(jumpedSourceIndex).getTimedCondition();
        // addInactiveClocks(jumpedSourceState.get(), it->second, jumpedSourceCondition);
        impreciseNeighbors.push(jumpedSourceState.get(), it->second, this->prefixes.at(jumpedSourceIndex));
        // We project to the non-exterior area
        const auto nonExteriorValuation = ExternalTransitionMaker::toValuation(jumpedSourceCondition);
        TATransition::Resets resetByContinuousExterior;
        for (std::size_t var = 0; var < nonExteriorValuation.size(); ++var) {
          resetByContinuousExterior.emplace_back(var, nonExteriorValuation.at(var));
        }
        if (sourceState == jumpedSourceState) {
          // Merge to the next discrete successor if it is a self loop
          for (const auto action: alphabet) {
            const auto jumpedSourceInvariant = this->prefixes.at(jumpedSourceIndex).getTimedCondition().toGuard();
            // Find a transition consistent with jumpedSourceInvariant
            auto transitionIt = std::find_if(jumpedSourceState->next.at(action).begin(),
                                             jumpedSourceState->next.at(action).end(),
                                             [&](const TATransition &transition) {
                                               return isWeaker(transition.guard, jumpedSourceInvariant);
                                             });
            assert(transitionIt != jumpedSourceState->next.at(action).end());
#ifdef DEBUG
            BOOST_LOG_TRIVIAL(debug) << "source: " << sourceState;
            BOOST_LOG_TRIVIAL(debug) << "jumpedSource: " << jumpedSourceState;
            BOOST_LOG_TRIVIAL(debug) << "target: " << transitionIt->target;
            BOOST_LOG_TRIVIAL(debug) << "resetByContinuousExterior: " << resetByContinuousExterior;
            BOOST_LOG_TRIVIAL(debug) << "resetByTransition: " << transitionIt->resetVars;
            BOOST_LOG_TRIVIAL(debug) << "composition: "
                                     << composition(transitionIt->resetVars, resetByContinuousExterior);
#endif
            const auto newReset = composition(transitionIt->resetVars, resetByContinuousExterior);
            // Add neighbors if we have imprecise clocks
            RenamingRelation renaming;
            renaming.reserve(newReset.size());
            for (const auto &[target, value]: newReset) {
              if (value.index() == 1) {
                renaming.emplace_back(std::make_pair(std::get<ClockVariables>(value), target));
              }
            }
            if (this->inP(this->discreteSuccessors.at(std::make_pair(jumpedSourceIndex, action)))) {
              impreciseNeighbors.push(transitionIt->target, renaming,
                                      this->prefixes.at(this->discreteSuccessors.at(std::make_pair(jumpedSourceIndex,
                                                                                                   action))));
            } else {
              const auto &map = this->closedRelation.at(
                      this->discreteSuccessors.at(std::make_pair(jumpedSourceIndex, action)));
              for (const auto &[mappedIndex, relation]: map) {
                if (this->inP(mappedIndex)) {
                  impreciseNeighbors.push(transitionIt->target, renaming, this->prefixes.at(mappedIndex));
                  break;
                }
              }
            }
            sourceState->next.at(action).emplace_back(transitionIt->target, newReset,
                                                      this->prefixes.at(continuousSuccessor)
                                                              .removeUpperBound().getTimedCondition().toGuard());
          }
        } else {
          ExternalTransitionMaker maker;
          maker.add(jumpedSourceState, it->second,
                    this->prefixes.at(continuousSuccessor).removeUpperBound().getTimedCondition(),
                    jumpedSourceCondition);
          assert(maker.make().size() == 1);
          // TODO: Consider adding neighbors
          sourceState->next[UNOBSERVABLE].push_back(maker.make().front());
        }
      }
#ifdef DEBUG
      BOOST_LOG_TRIVIAL(debug) << "as recognizable: " << this->toRecognizable();
      BOOST_LOG_TRIVIAL(info) << "Hypothesis before handling imprecise clocks\n" <<
                               TimedAutomaton{{states, {initialState}},
                                              TimedAutomaton::makeMaxConstants(states)}.simplify();
#endif
      impreciseNeighbors.run();
#ifdef DEBUG
      BOOST_LOG_TRIVIAL(debug) << "Hypothesis after handling imprecise clocks\n" <<
                               TimedAutomaton{{states, {initialState}},
                                              TimedAutomaton::makeMaxConstants(states)}.simplify();
      BOOST_LOG_TRIVIAL(debug) << "as recognizable: " << this->toRecognizable();
#endif
      // Make the transitions deterministic
      std::vector<TAState *> needSplit;
      needSplit.reserve(states.size());
      for (const auto &state: states) {
        state->removeTransitionsWithWeakerGuards();
        if (state->needSplitting()) {
          needSplit.push_back(state.get());
        }
      }
#ifdef DEBUG
      BOOST_LOG_TRIVIAL(debug) << "Hypothesis before state splitting\n"
                               << TimedAutomaton{{states, {initialState}},
                                                 TimedAutomaton::makeMaxConstants(states)}.simplify();
#endif
      // Conduct state splitting if necessary
      if (!needSplit.empty()) {
        BOOST_LOG_TRIVIAL(info) << "# of states before splitting: " << states.size();
        this->splitStates(states, initialState, needSplit);
        BOOST_LOG_TRIVIAL(info) << "# of states after splitting: " << states.size();
      }
      #ifdef DEBUG
      BOOST_LOG_TRIVIAL(debug) << "Hypothesis after state splitting\n"
                               << TimedAutomaton{{states, {initialState}},
                                                 TimedAutomaton::makeMaxConstants(states)}.simplify();
      #endif
      for (const auto &state: states) {
        state->mergeNondeterministicBranching();
      }
#ifdef DEBUG
      BOOST_LOG_TRIVIAL(debug) << "Hypothesis after making transitions deterministic\n" <<
                               TimedAutomaton{{states, {initialState}},
                                              TimedAutomaton::makeMaxConstants(states)}.simplify();
#endif

      // Assert the totality of the constructed DTA
      assert(std::all_of(this->pIndices.begin(), this->pIndices.end(), [&](std::size_t pIndex) {
        return !stateManager.isNew(pIndex);
      }));
      #if 0
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
      #endif
      for (std::size_t index = 0; index < this->prefixes.size(); index++) {
        if (stateManager.isNew(index)) {
          BOOST_LOG_TRIVIAL(warning) << "Partial transitions is detected";
          abort();
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

      return this->memOracle->printStatistics(stream);
    }
  };
}

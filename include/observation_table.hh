/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include <utility>
#include <vector>
#include <unordered_map>

#include "forward_regional_elementary_language.hh"
#include "backward_regional_elementary_language.hh"
#include "symbolic_membership_oracle.hh"
#include "timed_automaton.hh"
#include "equivalence.hh"

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
    std::unordered_map<std::size_t, std::unordered_map<std::size_t, std::vector<std::pair<std::size_t, std::size_t>>>> closedRelation;
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

    explicit ObservationTable(std::vector<Alphabet> alphabet, std::unique_ptr<SymbolicMembershipOracle> memOracle) :
            memOracle(std::move(memOracle)),
            alphabet(std::move(alphabet)),
            prefixes{ForwardRegionalElementaryLanguage{}},
            suffixes{BackwardRegionalElementaryLanguage{}} {
      pIndices.insert(0);
    }

  public:
    /*!
     * @brief Initialize the observation table
     */
    static ObservationTable
    initialize(std::vector<Alphabet> alphabet, std::unique_ptr<SymbolicMembershipOracle> memOracle) {
      ObservationTable observationTable{std::move(alphabet), std::move(memOracle)};
      observationTable.moveToP(0);
      observationTable.refreshTable();
      return observationTable;
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
     * @brief Construct a hypothesis DTA from the current timed observation table
     *
     * @pre The observation table is closed, consistent, and exterior-consistent.
     * @todo not implemented
     */
    TimedAutomaton generateHypothesis();
  };
}
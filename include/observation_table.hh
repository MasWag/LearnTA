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
    std::unordered_map<std::size_t, std::pair<std::size_t, std::vector<std::pair<std::size_t, std::size_t>>>> closedRelation;
    // The table containing the symbolic membership
    std::vector<std::vector<TimedConditionSet>> table;
    std::unordered_map<std::size_t, std::size_t> continuousSuccessors;
    std::unordered_map<std::pair<std::size_t, Alphabet>, std::size_t> discreteSuccessors;

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
      auto it = closedRelation.find(index);
      if (it != closedRelation.end()) {
        closedRelation.erase(it);
      }
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
     * @returns if the observation table is already closed
     * @todo Implement this
     */
    bool close() {
      abort();
    }

    /*!
     * @brief Make the observation table consistent
     *
     * @returns if the observation table is already consistent
     * @todo Implement this
     */
    bool consistent() {
      abort();
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
     * @todo not implemented
     */
    TimedAutomaton generateHypothesis();
  };
}
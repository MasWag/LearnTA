/**
 * @file equivalence.hh
 * @brief This file implements functions on the equivalence relation defined by the distinguishing suffixes
 * @author Masaki Waga
 * @date 2022/03/13
 */

#pragma once

#include "elementary_language.hh"
#include "backward_regional_elementary_language.hh"
#include "timed_condition_set.hh"
#include "juxtaposed_zone_set.hh"


namespace learnta {
  /*!
   * Return if two elementary languages are equivalent
   *
   * @pre leftRow.size() == rightRow.size() == suffixes.size()
   */
  bool equivalence(const ElementaryLanguage &left,
                   const std::vector<TimedConditionSet> &leftRow,
                   const ElementaryLanguage &right,
                   const std::vector<TimedConditionSet> &rightRow,
                   const std::vector<BackwardRegionalElementaryLanguage> &suffixes,
                   const std::vector<std::pair<std::size_t, std::size_t>> &renaming) {
    assert(leftRow.size() == rightRow.size());
    assert(rightRow.size() == suffixes.size());
    // Check the compatibility of prefixes up to renaming
    auto juxtaposition = left.getTimedCondition() ^ right.getTimedCondition();
    juxtaposition.addRenaming(renaming);
    if (!juxtaposition) {
      return false;
    }
    // Check the compatibility of symbolic membership up to renaming
    for (int i = 0; i < leftRow.size(); ++i) {
      const auto leftConcatenation = left + suffixes.at(i);
      const auto rightConcatenation = right + suffixes.at(i);
      auto leftJuxtaposition = JuxtaposedZoneSet{leftRow.at(i),
                                                 rightConcatenation.getTimedCondition(),
                                                 suffixes.at(i).wordSize() + 1};
      leftJuxtaposition.addRenaming(renaming);
      auto rightJuxtaposition = JuxtaposedZoneSet{leftConcatenation.getTimedCondition(),
                                                  rightRow.at(i),
                                                  suffixes.at(i).wordSize() + 1};
      rightJuxtaposition.addRenaming(renaming);
      if (leftJuxtaposition != rightJuxtaposition) {
        return false;
      }
    }

    return true;
  }

  /*!
   * Construct a renaming constraint if two elementary languages are equivalent
   *
   * The outline of our construction is as follows.
   * 1. We construct the bipartite graph based on the timed conditions.
   * 2. We make the set of the strictly constrained variables in the symbolic membership.
   * 3. We generate a candidate of renaming constraints and return it if it witnesses the equivalence
   *
   * In the first part, we construct a bipartite graph \f$(V_1, V_2, E)\f$ such that:
   * - \f$V_1\f$ and \f$V_2\f$ consists of the variables in left and right, respectively, and
   * - \f$(v_1, v_2) \in E\f$ if and only if \f$v_1 = v_2\f$ does not contradict to the given constraints.
   *
   * We note that each disjoint part of this bipartite graph is complete.
   *
   * In the second part, we list the variables strictly constrained in the symbolic membership.
   * We do this simply by comparing the coefficients in the zone thanks to the canonicity of the zones.
   * Since these strictly constrained variables must also be constrained by the renaming constraints, they are the candidates in the construction of the renaming constraints.
   *
   * In the third part, we construct the renaming constraints by taking the largest or smallest edges in each disjoint part of the bipartite graph.
   *
   * @pre leftRow.size() == rightRow.size() == suffixes.size()
   * @pre left and right are simple
   * @todo Write the test
   */
  std::optional<std::vector<std::pair<std::size_t, std::size_t>>>
  findEquivalentRenaming(const ElementaryLanguage &left,
                         const std::vector<TimedConditionSet> &leftRow,
                         const ElementaryLanguage &right,
                         const std::vector<TimedConditionSet> &rightRow,
                         const std::vector<BackwardRegionalElementaryLanguage> &suffixes) {
    // 0. Asserts the preconditions
    assert(leftRow.size() == rightRow.size());
    assert(rightRow.size() == suffixes.size());
    assert(left.isSimple());
    assert(right.isSimple());

    // 1. Construct the bipartite graph based on the timed conditions.
    std::vector<std::vector<std::size_t>> v1Edges, v2Edges;
    const auto N = left.wordSize() + 1;
    const auto M = right.wordSize() + 1;
    v1Edges.resize(N);
    v2Edges.resize(M);
    std::size_t v1 = 0, v2 = 0;
    std::vector<std::size_t> currentSameV1, currentSameV2;
    while (v1 < v1Edges.size() && v2 < v2Edges.size()) {
      if (left.getTimedCondition().getUpperBound(v1, N) == right.getTimedCondition().getUpperBound(v2, N)) {
        v1Edges.reserve(currentSameV2.size());
        v2Edges.reserve(currentSameV1.size());
        std::copy(currentSameV2.begin(), currentSameV2.end(), std::back_inserter(v1Edges.at(v1)));
        std::for_each(currentSameV2.begin(), currentSameV2.end(), [&](const auto oldV2) {
          v2Edges.at(oldV2).push_back(v1);
        });
        std::copy(currentSameV1.begin(), currentSameV1.end(), std::back_inserter(v2Edges.at(v1)));
        std::for_each(currentSameV1.begin(), currentSameV1.end(), [&](const auto oldV1) {
          v1Edges.at(oldV1).push_back(v2);
        });
        v1Edges.at(v1).push_back(v2);
        v2Edges.at(v2).push_back(v1);
        currentSameV1.push_back(v1);
        currentSameV2.push_back(v2);
      } else {
        currentSameV1.clear();
        currentSameV2.clear();
        if (left.getTimedCondition().getUpperBound(v1, N) < right.getTimedCondition().getUpperBound(v2, N)) {
          v2++;
        } else {
          v1++;
        }
      }
    }

    // 2. Make the set of the strictly constrained variables in the symbolic membership.
    std::vector<std::size_t> constrainedV1, constrainedV2;
    for (int i = 0; i < leftRow.size(); ++i) {
      const auto leftConcatenation = left + suffixes.at(i);
      const auto rightConcatenation = right + suffixes.at(i);
      // 2-1. We quickly check if they are clearly inequivalent.
      if ((leftRow.at(i).empty() && !rightRow.at(i).empty()) || (!leftRow.at(i).empty() && rightRow.at(i).empty())) {
        // One of them is bottom but another is not
        return std::nullopt;
      }
      auto currentV1Constrained =
              leftRow.at(i).getStrictlyConstrainedVariables(leftConcatenation.getTimedCondition(), N - 1);
      auto currentV2Constrained =
              rightRow.at(i).getStrictlyConstrainedVariables(rightConcatenation.getTimedCondition(), M - 1);
      if ((currentV1Constrained.empty() && !currentV2Constrained.empty()) ||
          (!currentV1Constrained.empty() && currentV2Constrained.empty())) {
        // One of them is trivial but another is not
        return std::nullopt;
      }
      std::move(currentV1Constrained.begin(), currentV1Constrained.end(), std::back_inserter(constrainedV1));
      std::move(currentV2Constrained.begin(), currentV2Constrained.end(), std::back_inserter(constrainedV2));
    }
    std::sort(constrainedV1.begin(), constrainedV1.end());
    constrainedV1.erase(std::unique(constrainedV1.begin(), constrainedV1.end()), constrainedV1.end());
    std::sort(constrainedV2.begin(), constrainedV2.end());
    constrainedV2.erase(std::unique(constrainedV2.begin(), constrainedV2.end()), constrainedV2.end());

    // 3. Generate candidate renaming
    std::vector<std::vector<std::pair<std::size_t, std::size_t>>> candidates;
    candidates.emplace_back();
    {
      auto v1Index = 0, v2Index = 0;
      while (v1Index < constrainedV1.size() && v2Index < constrainedV2.size()) {
        v1 = constrainedV1.at(v1Index);
        v2 = constrainedV2.at(v2Index);
        // 3-1. We quickly check if we cannot construct any candidate
        if (!std::binary_search(v1Edges.at(v1).begin(), v1Edges.at(v1).end(), v2)) {
          return std::nullopt;
        }
        if (!std::binary_search(v2Edges.at(v2).begin(), v2Edges.at(v2).end(), v1)) {
          return std::nullopt;
        }
        const auto smallestEdge = std::make_pair(v1, v2);
        const auto largestEdge = std::make_pair(v1Edges.at(v1).back(), v2Edges.at(v2).back());
        std::vector<std::vector<std::pair<std::size_t, std::size_t>>> tmp;
        tmp.reserve(candidates.size() * 2);
        for (const auto &candidate: candidates) {
          tmp.push_back(candidate);
          tmp.back().push_back(smallestEdge);
          tmp.push_back(candidate);
          tmp.back().push_back(largestEdge);
        }
        candidates.resize(tmp.size());
        std::move(tmp.begin(), tmp.end(), candidates.begin());
        // We increment the indices until reaching the next disjoint part of the bipartite graph
        while (v1Index < constrainedV1.size() && constrainedV1.at(v1Index) <= v2Edges.at(v2).back()) {
          v1Index++;
        }
        while (v2Index < constrainedV2.size() && constrainedV2.at(v1Index) <= v1Edges.at(v1).back()) {
          v2Index++;
        }
      }
    }

    // 4. Check candidate renaming
    auto it = std::find_if(candidates.begin(), candidates.end(), [&] (const auto& candidate) {
      return equivalence(left, leftRow, right, rightRow, suffixes, candidate);
    });
    if (it == candidates.end()) {
      return std::nullopt;
    } else {
      return std::make_optional(*it);
    }
  }
}
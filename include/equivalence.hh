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
#include "renaming_relation.hh"

namespace learnta {
  /*!
   * @brief Return if two elementary languages are equivalent
   *
   * @pre leftRow.size() == rightRow.size() == suffixes.size()
   */
  static bool equivalence(const ElementaryLanguage &left,
                          const std::vector<TimedConditionSet> &leftRow,
                          const ElementaryLanguage &right,
                          const std::vector<TimedConditionSet> &rightRow,
                          const std::vector<BackwardRegionalElementaryLanguage> &suffixes,
                          const RenamingRelation &renaming) {
#ifdef LEARNTA_DEBUG_EQUIVALENCE
    BOOST_LOG_TRIVIAL(trace) << "left: " << left;
    BOOST_LOG_TRIVIAL(trace) << "right: " << right;
    BOOST_LOG_TRIVIAL(trace) << "leftRowSize: " << leftRow.back().size();
    // BOOST_LOG_TRIVIAL(trace) << "leftRowBack: " << leftRow.back().front();
    BOOST_LOG_TRIVIAL(trace) << "rightRowSize: " << rightRow.back().size();
    BOOST_LOG_TRIVIAL(trace) << "suffix.back(): " << suffixes.back();
#endif
    assert(leftRow.size() == rightRow.size());
    assert(rightRow.size() == suffixes.size());
    // Check the compatibility of prefixes up to renaming
    auto juxtaposition = left.getTimedCondition() ^ right.getTimedCondition();
    juxtaposition.addRenaming(renaming);
    if (!juxtaposition.isSatisfiableNoCanonize()) {
      return false;
    }
    // Check the compatibility of symbolic membership up to renaming
    for (std::size_t i = 0; i < leftRow.size(); ++i) {
      const auto leftConcatenation = left + suffixes.at(i);
      const auto rightConcatenation = right + suffixes.at(i);
      JuxtaposedZoneSet leftJuxtaposition {leftRow.at(i),
                                           rightConcatenation.getTimedCondition(),
                                           suffixes.at(i).wordSize()};
      leftJuxtaposition.addRenaming(renaming);
      JuxtaposedZoneSet rightJuxtaposition {leftConcatenation.getTimedCondition(),
                                            rightRow.at(i),
                                            suffixes.at(i).wordSize()};
      rightJuxtaposition.addRenaming(renaming);
      if (leftJuxtaposition != rightJuxtaposition) {
        return false;
      }
    }

    return true;
  }

  /*!
   * @brief Return if two elementary languages are equivalent
   *
   * @pre leftRow.size() == rightRow.size() == suffixes.size()
   */
  static bool equivalence(const ElementaryLanguage &left,
                          const std::vector<TimedConditionSet> &leftRow,
                          const std::vector<TimedCondition>& leftConcatenation,
                          const ElementaryLanguage &right,
                          const std::vector<TimedConditionSet> &rightRow,
                          const std::vector<TimedCondition>& rightConcatenation,
                          const std::vector<BackwardRegionalElementaryLanguage> &suffixes,
                          const RenamingRelation &renaming) {
#ifdef LEARNTA_DEBUG_EQUIVALENCE
    BOOST_LOG_TRIVIAL(trace) << "left: " << left;
    BOOST_LOG_TRIVIAL(trace) << "right: " << right;
    BOOST_LOG_TRIVIAL(trace) << "leftRowSize: " << leftRow.back().size();
    // BOOST_LOG_TRIVIAL(trace) << "leftRowBack: " << leftRow.back().front();
    BOOST_LOG_TRIVIAL(trace) << "rightRowSize: " << rightRow.back().size();
    BOOST_LOG_TRIVIAL(trace) << "suffix.back(): " << suffixes.back();
#endif
    assert(leftRow.size() == rightRow.size());
    assert(rightRow.size() == suffixes.size());
    // Check the compatibility of prefixes up to renaming
    auto juxtaposition = left.getTimedCondition() ^ right.getTimedCondition();
    juxtaposition.addRenaming(renaming);
    if (!juxtaposition.isSatisfiableNoCanonize()) {
      return false;
    }
    // Check the compatibility of symbolic membership up to renaming
    for (std::size_t i = 0; i < leftRow.size(); ++i) {
      JuxtaposedZoneSet leftJuxtaposition{leftRow.at(i), rightConcatenation.at(i), suffixes.at(i).wordSize()};
      leftJuxtaposition.addRenaming(renaming);
      JuxtaposedZoneSet rightJuxtaposition{leftConcatenation.at(i), rightRow.at(i), suffixes.at(i).wordSize()};
      rightJuxtaposition.addRenaming(renaming);
      if (leftJuxtaposition != rightJuxtaposition) {
        return false;
      }
    }

    return true;
  }

  /*!
   * @brief Return if two elementary languages are equivalent
   *
   * @param leftRightJuxtaposition juxtaposition of left and right prefixes
   * @param leftJuxtapositions list of juxtaposition of mem(left + suffix) and (right + suffix)
   * @param rightJuxtapositions list of juxtaposition of (left + suffix) and mem(right + suffix)
   *
   * @pre leftJuxtapositions.size() == rightJuxtapositions.size()
   */
   static bool equivalence(JuxtaposedZone leftRightJuxtaposition,
                           const std::vector<JuxtaposedZoneSet> &leftJuxtapositions,
                           const std::vector<JuxtaposedZoneSet> &rightJuxtapositions,
                           const RenamingRelation &renaming) {
     assert(leftJuxtapositions.size() == rightJuxtapositions.size());
     // Check the compatibility of prefixes up to renaming
     leftRightJuxtaposition.addRenaming(renaming);
     if (!leftRightJuxtaposition) {
       return false;
     }
     // Check the compatibility of symbolic membership up to renaming
     return std::equal(leftJuxtapositions.begin(), leftJuxtapositions.end(),
                       rightJuxtapositions.begin(), rightJuxtapositions.end(),
                       [&renaming] (JuxtaposedZoneSet left, JuxtaposedZoneSet right) {
       left.addRenaming(renaming);
       right.addRenaming(renaming);
       return left == right;
     });
   }

   using RenamingGraph = std::pair<std::vector<std::vector<std::size_t>>, std::vector<std::vector<std::size_t>>>;

   /*!
    * @brief Construct the intermediate bipartite graph for candidate generation from timed conditions
    *
    * The constructed graph has an edge between v1.at(i) and v2.at(j) if and only if
    *  left.getUpperBound(i, N - 1) == right.getUpperBound(j, M - 1)
    *
    * @pre left and right are simple
    */
   static inline RenamingGraph toGraph(const TimedCondition &left, const TimedCondition &right) {
     // Assert the precondition
     assert(left.isSimple());
     assert(right.isSimple());
     std::vector<std::vector<std::size_t>> v1Edges, v2Edges;

     const auto N = left.size();
     const auto M = right.size();
     v1Edges.resize(N);
     v2Edges.resize(M);
     std::size_t v1 = 0, v2 = 0;
     std::vector<std::size_t> currentSameV1, currentSameV2;
     while (v1 < v1Edges.size() && v2 < v2Edges.size()) {
       if (left.getUpperBound(v1, N - 1) == right.getUpperBound(v2, M - 1)) {
         v1Edges.reserve(currentSameV2.size());
         v2Edges.reserve(currentSameV1.size());
         std::copy(currentSameV2.begin(), currentSameV2.end(), std::back_inserter(v1Edges.at(v1)));
         std::for_each(currentSameV2.begin(), currentSameV2.end(), [&](const auto oldV2) {
           v2Edges.at(oldV2).push_back(v1);
         });
         std::copy(currentSameV1.begin(), currentSameV1.end(), std::back_inserter(v2Edges.at(v2)));
         std::for_each(currentSameV1.begin(), currentSameV1.end(), [&](const auto oldV1) {
           v1Edges.at(oldV1).push_back(v2);
         });
         v1Edges.at(v1).push_back(v2);
         v2Edges.at(v2).push_back(v1);
         currentSameV1.push_back(v1);
         currentSameV2.push_back(v2);
         if (v1 + 1 < v1Edges.size() && left.getUpperBound(v1, N - 1) == left.getUpperBound(v1 + 1, N - 1)) {
           v1++;
         } else {
           v2++;
         }
       } else {
         currentSameV1.clear();
         currentSameV2.clear();
         if (left.getUpperBound(v1, N - 1) < right.getUpperBound(v2, M - 1)) {
           v2++;
         } else {
           v1++;
         }
       }
     }

     // Sort and remove duplication
     for (auto& v1Edge: v1Edges) {
       std::sort(v1Edge.begin(), v1Edge.end());
       v1Edge.erase(std::unique(v1Edge.begin(), v1Edge.end()), v1Edge.end());
     }
     for (auto& v2Edge: v2Edges) {
       std::sort(v2Edge.begin(), v2Edge.end());
       v2Edge.erase(std::unique(v2Edge.begin(), v2Edge.end()), v2Edge.end());
     }
     return RenamingGraph{v1Edges, v2Edges};
   }

   /*!
    * @brief Construct candidate renaming relations that are deterministic, i.e., the corresponding reset only makes precise clocks
    *
    * @param left The left timed condition in the renaming
    * @param right The right timed condition in the renaming
    * @param leftConstrained The variables strictly constrained in mem(left ・ suffix)
    * @param rightConstrained The variables strictly constrained in mem(right ・ suffix)
    * @param graph The renaming graph
    *
    * @pre leftConstrained and rightConstrained are strictly ascending.
    */
   static inline std::vector<RenamingRelation> generateDeterministicCandidates(const TimedCondition& left,
                                                                               const TimedCondition& right,
                                                                               const std::vector<std::size_t>& leftConstrained,
                                                                               const std::vector<std::size_t>& rightConstrained,
                                                                               const RenamingGraph& graph) {
     // Assert the preconditions
     assert(is_strict_ascending(leftConstrained));
     assert(is_strict_ascending(rightConstrained));
     std::vector<RenamingRelation> candidates;
     // We first generate full candidates
     candidates.emplace_back();
     for (std::size_t j = 0; j < right.size(); ++j) {
       // We do not construct a renaming equation if there is no edge from the right node, or the target is already determined
       if (graph.second.at(j).empty() || right.getUpperBound(j, right.size() - 1).second) {
         continue;
       }
       std::vector<RenamingRelation> newCandidates;
       for (auto candidate: candidates) {
         // If j - 1 and j has the same value, we use the same left variable
         if (j > 0 && right.getUpperBound(j - 1, j - 1) == Bounds{0, true}) {
           candidate.emplace_back(candidate.back().first, j);
           newCandidates.emplace_back(std::move(candidate));
           continue;
         } else {
           // The least value of the corresponding node
           const std::size_t lowerBound = candidate.empty() ? 0 : (candidate.back().first + 1);
           const bool constrained = std::binary_search(rightConstrained.begin(), rightConstrained.end(), j);
           for (const auto& i: graph.second.at(j)) {
             // We skip if i - 1 and i has the same value
             if (i >= lowerBound && constrained == std::binary_search(leftConstrained.begin(), leftConstrained.end(), i) &&
                 (i == 0 || left.getUpperBound(i - 1, i - 1) != Bounds{0, true})) {
               auto tmpCandidate = candidate;
               tmpCandidate.emplace_back(i, j);
               newCandidates.emplace_back(std::move(tmpCandidate));
             }
           }
         }
       }
       candidates = std::move(newCandidates);
     }

     // Then, we add empty renaming
     candidates.emplace_back();
     // Finally, erase non-deterministic renaming
     candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [&] (const auto& renaming) {
       return renaming.hasImpreciseClocks(right);
     }), candidates.end());
     std::sort(candidates.begin(), candidates.end());
     candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

     return candidates;
   }

   /*!
    * @brief The status of a cell of the observation table
    *
    * - CellStatus::bottom: \f$mem(p \cdot s) = \bot\f$
    * - CellStatus::top: \f$mem(p \cdot s) = \top\f$
    * - CellStatus::middle: otherwise
    */
   enum class CellStatus {
       bottom,
       top,
       middle
   };

   /*!
    * @brief Compute the status of a cell of the observation table
    *
    * @pre If concatenation == cell, we have cell.size() == 1 and concatenation == cell.front()
    */
   static inline CellStatus decideStatus(const TimedCondition &concatenation, const TimedConditionSet &cell) {
       // Assert the precondition
#ifndef NDEBUG
       const auto simpleConcatenations = concatenation.enumerate();
       assert((cell.size() == 1 && cell.getConditions().front() == concatenation) ||
              std::any_of(simpleConcatenations.begin(), simpleConcatenations.end(), [&] (const TimedCondition& simple) {
                  return std::none_of(cell.getConditions().begin(), cell.getConditions().end(),
                                      [&] (const TimedCondition& disjunct) {
                      return disjunct.includes(simple);
                  });
              }));
#endif

       if (cell.empty()) {
           return CellStatus::bottom;
       } else if (cell.size() == 1 && cell.getConditions().front() == concatenation) {
           return CellStatus::top;
       } else {
           return CellStatus::middle;
       }
   }

   /*!
    * @brief Return the constrained variables in the symbolic membership
    *
    * @param leftRow mem(left ・ suffix)
    * @param rightRow mem(right ・ suffix)
    * @param leftConcatenations left ・ suffix
    * @param rightConcatenations right ・ suffix
    * @param N |left|
    * @param M |right|
    *
    * @return The constrained variables in strictly ascending order
    */
   static inline std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
   makeConstrainedVariables(const std::vector<TimedConditionSet> &leftRow,
                            const std::vector<TimedConditionSet> &rightRow,
                            const std::vector<TimedCondition> &leftConcatenations,
                            const std::vector<TimedCondition> &rightConcatenations,
                            const std::size_t N,
                            const std::size_t M) {
       std::vector<std::size_t> constrainedV1, constrainedV2;
       constrainedV1.reserve(N * leftRow.size());
       constrainedV2.reserve(M * rightRow.size());
       for (std::size_t i = 0; i < leftRow.size(); ++i) {
           auto currentV1Constrained =
                   leftRow.at(i).getStrictlyConstrainedVariables(leftConcatenations.at(i), N);
           auto currentV2Constrained =
                   rightRow.at(i).getStrictlyConstrainedVariables(rightConcatenations.at(i), M);
           std::move(currentV1Constrained.begin(), currentV1Constrained.end(), std::back_inserter(constrainedV1));
           std::move(currentV2Constrained.begin(), currentV2Constrained.end(), std::back_inserter(constrainedV2));
       }
       std::sort(constrainedV1.begin(), constrainedV1.end());
       constrainedV1.erase(std::unique(constrainedV1.begin(), constrainedV1.end()), constrainedV1.end());
       std::sort(constrainedV2.begin(), constrainedV2.end());
       constrainedV2.erase(std::unique(constrainedV2.begin(), constrainedV2.end()), constrainedV2.end());

       assert(is_strict_ascending(constrainedV1));
       assert(is_strict_ascending(constrainedV2));

       return std::make_pair(constrainedV1, constrainedV2);
   }

  /*!
   * @brief Return a renaming constraint if two elementary languages are equivalent
   *
   * The outline of our construction is as follows.
   * 1. We construct the bipartite graph based on the timed conditions.
   * 2. We generate deterministic candidate renaming equations
   * 3. We return a renaming equation witnessing the equivalence, if exists
   *
   * In the first part, we construct a bipartite graph \f$(V_1, V_2, E)\f$ such that:
   * - \f$V_1\f$ and \f$V_2\f$ consists of the variables in left and right, respectively, and
   * - \f$(v_1, v_2) \in E\f$ if and only if \f$v_1 = v_2\f$ does not contradict to the given constraints.
   *
   * We note that each disjoint part of this bipartite graph is complete.
   *
   *
   * @pre leftRow.size() == rightRow.size() == suffixes.size()
   * @pre left and right are simple
   */
  static inline std::optional<RenamingRelation> findDeterministicEquivalentRenaming(const ElementaryLanguage &left,
                                                                                    const std::vector<TimedConditionSet> &leftRow,
                                                                                    const ElementaryLanguage &right,
                                                                                    const std::vector<TimedConditionSet> &rightRow,
                                                                                    const std::vector<BackwardRegionalElementaryLanguage> &suffixes) {
    // 0. Asserts the preconditions
    assert(leftRow.size() == rightRow.size());
    assert(rightRow.size() == suffixes.size());
    assert(left.isSimple());
    assert(right.isSimple());

    // 0.1. We quickly check if they are clearly not equivalent.
    if (!std::equal(leftRow.begin(), leftRow.end(), rightRow.begin(), rightRow.end(),
                    [&] (const auto& leftCell, const auto& rightCell) {
                        return leftCell.empty() == rightCell.empty();
                    })) {
        return std::nullopt;
    }

    // 0.2 Compute the concatenations
    // left + suffix
    std::vector<TimedCondition> leftConcatenations;
    leftConcatenations.reserve(suffixes.size());
    std::transform(suffixes.begin(), suffixes.end(), std::back_inserter(leftConcatenations), [&] (const auto& suffix) {
        return left.getTimedCondition() + suffix.getTimedCondition();
    });
    // right + suffix
    std::vector<TimedCondition> rightConcatenations;
    rightConcatenations.reserve(suffixes.size());
    std::transform(suffixes.begin(), suffixes.end(), std::back_inserter(rightConcatenations), [&] (const auto& suffix) {
        return right.getTimedCondition() + suffix.getTimedCondition();
    });

    // 0.3 Compute the status
    std::vector<CellStatus> leftStatus;
    leftStatus.reserve(suffixes.size());
    std::transform(leftConcatenations.begin(), leftConcatenations.end(),
                   leftRow.begin(), std::back_inserter(leftStatus), decideStatus);
    std::vector<CellStatus> rightStatus;
    rightStatus.reserve(suffixes.size());
    std::transform(rightConcatenations.begin(), rightConcatenations.end(),
                   rightRow.begin(), std::back_inserter(rightStatus), decideStatus);

    // 0.4. We quickly check if their statuses are not equivalent.
    if (!std::equal(leftStatus.begin(), leftStatus.end(), rightStatus.begin(), rightStatus.end())) {
      return std::nullopt;
    }

    // 1. Try the empty relation
    if (equivalence(left, leftRow, leftConcatenations,
                    right, rightRow, rightConcatenations,
                    suffixes, RenamingRelation{})) {
      return RenamingRelation{};
    }

    // 2. Construct the bipartite graph based on the timed conditions.
    const auto graph = toGraph(left.getTimedCondition(), right.getTimedCondition());

    // 3. Construct the strictly constrained variables
    const auto& [leftConstrained, rightConstrained] = makeConstrainedVariables(leftRow, rightRow,
                                                                               leftConcatenations, rightConcatenations,
                                                                               left.wordSize() + 1,
                                                                               right.wordSize() + 1);
    // 4. Construct the candidate renaming equations
    auto candidates = generateDeterministicCandidates(left.getTimedCondition(),
                                                      right.getTimedCondition(),
                                                      leftConstrained,
                                                      rightConstrained,
                                                      graph);

    // 5. Find an equivalent renaming equation
    const auto leftRightJuxtaposition = left.getTimedCondition() ^ right.getTimedCondition();
    // Add implicit constraints
    std::for_each(candidates.begin(), candidates.end(), [&] (auto &candidate) {
      if (!candidate.empty()) {
        candidate.addImplicitConstraints(leftRightJuxtaposition);
      }
    });
    std::vector<JuxtaposedZoneSet> leftJuxtapositions, rightJuxtapositions;
    leftJuxtapositions.reserve(leftRow.size());
    rightJuxtapositions.reserve(rightRow.size());
    for (std::size_t i = 0; i < leftRow.size(); ++i) {
      leftJuxtapositions.emplace_back(leftRow.at(i), rightConcatenations.at(i), suffixes.at(i).wordSize());
      rightJuxtapositions.emplace_back(leftConcatenations.at(i), rightRow.at(i), suffixes.at(i).wordSize());
    }
    auto it = std::find_if(candidates.begin(), candidates.end(), [&](const auto &candidate) {
      return equivalence(leftRightJuxtaposition, leftJuxtapositions, rightJuxtapositions, candidate);
    });

    if (it != candidates.end()) {
      return *it;
    } else {
      return std::nullopt;
    }
  }

  /*!
   * @brief Return a renaming constraint if two elementary languages are equivalent
   *
   * The outline of our construction is as follows.
   * 1. We construct the bipartite graph based on the timed conditions.
   * 2. We generate deterministic candidate renaming equations
   * 3. We return a renaming equation witnessing the equivalence, if exists
   *
   * In the first part, we construct a bipartite graph \f$(V_1, V_2, E)\f$ such that:
   * - \f$V_1\f$ and \f$V_2\f$ consists of the variables in left and right, respectively, and
   * - \f$(v_1, v_2) \in E\f$ if and only if \f$v_1 = v_2\f$ does not contradict to the given constraints.
   *
   * We note that each disjoint part of this bipartite graph is complete.
   *
   *
   * @pre leftRow.size() == rightRow.size() == suffixes.size()
   * @pre left and right are simple
   */
  static inline std::optional<RenamingRelation> findDeterministicEquivalentRenaming(const ElementaryLanguage &left,
                                                                                    const std::vector<TimedConditionSet> &leftRow,
                                                                                    const std::vector<TimedCondition>& leftConcatenations,
                                                                                    const ElementaryLanguage &right,
                                                                                    const std::vector<TimedConditionSet> &rightRow,
                                                                                    const std::vector<TimedCondition>& rightConcatenations,
                                                                                    const std::vector<BackwardRegionalElementaryLanguage> &suffixes) {
    // 0. Asserts the preconditions
    assert(leftRow.size() == rightRow.size());
    assert(rightRow.size() == suffixes.size());
    assert(left.isSimple());
    assert(right.isSimple());

    // 0.1. Compute the status
    std::vector<CellStatus> leftStatus;
    leftStatus.reserve(suffixes.size());
    std::transform(leftConcatenations.begin(), leftConcatenations.end(),
                   leftRow.begin(), std::back_inserter(leftStatus), decideStatus);
    std::vector<CellStatus> rightStatus;
    rightStatus.reserve(suffixes.size());
    std::transform(rightConcatenations.begin(), rightConcatenations.end(),
                   rightRow.begin(), std::back_inserter(rightStatus), decideStatus);

    // 0.2. We quickly check if their statuses are not equivalent.
    if (!std::equal(leftStatus.begin(), leftStatus.end(), rightStatus.begin(), rightStatus.end())) {
      return std::nullopt;
    }

    // 1. Try the empty relation
    if (equivalence(left, leftRow, leftConcatenations,
                    right, rightRow, rightConcatenations,
                    suffixes, RenamingRelation{})) {
      return RenamingRelation{};
    }

    // 2. Construct the bipartite graph based on the timed conditions.
    const auto graph = toGraph(left.getTimedCondition(), right.getTimedCondition());

    // 3. Construct the strictly constrained variables
    const auto& [leftConstrained, rightConstrained] = makeConstrainedVariables(leftRow, rightRow,
                                                                               leftConcatenations, rightConcatenations,
                                                                               left.wordSize() + 1,
                                                                               right.wordSize() + 1);
    // 4. Construct the candidate renaming equations
    auto candidates = generateDeterministicCandidates(left.getTimedCondition(),
                                                      right.getTimedCondition(),
                                                      leftConstrained,
                                                      rightConstrained,
                                                      graph);

    // 5. Find an equivalent renaming equation
    const auto leftRightJuxtaposition = left.getTimedCondition() ^ right.getTimedCondition();
    // Add implicit constraints
    std::for_each(candidates.begin(), candidates.end(), [&] (auto &candidate) {
      if (!candidate.empty()) {
        candidate.addImplicitConstraints(leftRightJuxtaposition);
      }
    });
    std::vector<JuxtaposedZoneSet> leftJuxtapositions, rightJuxtapositions;
    leftJuxtapositions.reserve(leftRow.size());
    rightJuxtapositions.reserve(rightRow.size());
    for (std::size_t i = 0; i < leftRow.size(); ++i) {
      leftJuxtapositions.emplace_back(leftRow.at(i), rightConcatenations.at(i), suffixes.at(i).wordSize());
      rightJuxtapositions.emplace_back(leftConcatenations.at(i), rightRow.at(i), suffixes.at(i).wordSize());
    }
    auto it = std::find_if(candidates.begin(), candidates.end(), [&](const auto &candidate) {
      return equivalence(leftRightJuxtaposition, leftJuxtapositions, rightJuxtapositions, candidate);
    });

    if (it != candidates.end()) {
      return *it;
    } else {
      return std::nullopt;
    }
  }

    /*!
     * @brief Construct a renaming constraint if two elementary languages are equivalent
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
     */
  static inline std::optional<RenamingRelation> findEquivalentRenaming(const ElementaryLanguage &left,
                                                                       const std::vector<TimedConditionSet> &leftRow,
                                                                       const ElementaryLanguage &right,
                                                                       const std::vector<TimedConditionSet> &rightRow,
                                                                       const std::vector<BackwardRegionalElementaryLanguage> &suffixes) {
    // 0. Asserts the preconditions
    assert(leftRow.size() == rightRow.size());
    assert(rightRow.size() == suffixes.size());
    assert(left.isSimple());
    assert(right.isSimple());

    // 0.5. Try the empty relation
    if (equivalence(left, leftRow, right, rightRow, suffixes, RenamingRelation{})) {
      return RenamingRelation{};
    }

    // 1. Construct the bipartite graph based on the timed conditions.
    const auto graph = toGraph(left.getTimedCondition(), right.getTimedCondition());
    const auto &v1Edges = graph.first;
    const auto &v2Edges = graph.second;
    const auto N = left.wordSize() + 1;
    const auto M = right.wordSize() + 1;

    std::vector<TimedCondition> leftConcatenations, rightConcatenations;
    leftConcatenations.resize(leftRow.size());
    rightConcatenations.resize(leftRow.size());
    // 2. Make the set of the strictly constrained variables in the symbolic membership.
    std::vector<std::size_t> constrainedV1, constrainedV2;
    for (std::size_t i = 0; i < leftRow.size(); ++i) {
      leftConcatenations.at(i) = (left + suffixes.at(i)).getTimedCondition();
      rightConcatenations.at(i) = (right + suffixes.at(i)).getTimedCondition();
      // 2-1. We quickly check if they are clearly not equivalent.
      if (leftRow.at(i).empty() != rightRow.at(i).empty()) {
        // One of them is bottom but another is not
        return std::nullopt;
      }
      auto currentV1Constrained =
              leftRow.at(i).getStrictlyConstrainedVariables(leftConcatenations.at(i), N);
      auto currentV2Constrained =
              rightRow.at(i).getStrictlyConstrainedVariables(rightConcatenations.at(i), M);
      std::move(currentV1Constrained.begin(), currentV1Constrained.end(), std::back_inserter(constrainedV1));
      std::move(currentV2Constrained.begin(), currentV2Constrained.end(), std::back_inserter(constrainedV2));
    }
    std::sort(constrainedV1.begin(), constrainedV1.end());
    constrainedV1.erase(std::unique(constrainedV1.begin(), constrainedV1.end()), constrainedV1.end());
    constrainedV1.erase(
            std::remove_if(constrainedV1.begin(), constrainedV1.end(), [&](const auto &v1) {
              return v1Edges.at(v1).empty();
            }), constrainedV1.end());
    std::sort(constrainedV2.begin(), constrainedV2.end());
    constrainedV2.erase(std::unique(constrainedV2.begin(), constrainedV2.end()), constrainedV2.end());
    constrainedV2.erase(
            std::remove_if(constrainedV2.begin(), constrainedV2.end(), [&](const auto &v2) {
              return v2Edges.at(v2).empty();
            }), constrainedV2.end());

    // 3. Generate candidate renaming
    std::deque<RenamingRelation> candidates;
    candidates.emplace_back();
    {
      std::size_t v1Index = 0, v2Index = 0;
      while (v1Index < constrainedV1.size() && v2Index < constrainedV2.size()) {
        std::size_t v1 = constrainedV1.at(v1Index);
        std::size_t v2 = constrainedV2.at(v2Index);
        // 3-1. We check if we cannot include these vertices
        if (v1Edges.at(v1).empty()) {
          v1Index++;
          // We cannot conclude that they are not equivalent because this non-trivial constraint can be realized from other non-trivial constraints.
          continue;
        }
        if (v2Edges.at(v2).empty()) {
          v2Index++;
          continue;
        }
        if (!std::binary_search(v1Edges.at(v1).begin(), v1Edges.at(v1).end(), v2)) {
          if (v1Edges.at(v1).back() >= v2) {
            v2Index++;
          } else {
            v1Index++;
          }
          continue;
        }
        if (!std::binary_search(v2Edges.at(v2).begin(), v2Edges.at(v2).end(), v1)) {
          if (v2Edges.at(v2).back() >= v1) {
            v1Index++;
          } else {
            v2Index++;
          }
          continue;
        }
        std::vector<RenamingRelation> tmp;
        tmp.reserve(candidates.size() * v2Edges.at(v2).size() * v1Edges.at(v1).size());

        for (const auto currentV1: v2Edges.at(v2)) {
          for (const auto currentV2: v1Edges.at(v1)) {
            const auto currentEdge = std::make_pair(currentV1, currentV2);
            for (const auto &candidate: candidates) {
              tmp.emplace_back(candidate);
              tmp.emplace_back(candidate);
              tmp.back().emplace_back(currentEdge);
              tmp.emplace_back();
              tmp.back().emplace_back(currentEdge);
            }
          }
        }

        candidates.resize(tmp.size());
        std::move(tmp.begin(), tmp.end(), candidates.begin());
        // We increment the indices until reaching the next disjoint part of the bipartite graph
        while (v1Index < constrainedV1.size() && constrainedV1.at(v1Index) <= v2Edges.at(v2).back()) {
          v1Index++;
        }
        while (v2Index < constrainedV2.size() && constrainedV2.at(v2Index) <= v1Edges.at(v1).back()) {
          v2Index++;
        }
      }
    }

    // Reduce the candidates for optimization
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

    // 4. Check candidate renaming
    const auto leftRightJuxtaposition = left.getTimedCondition() ^ right.getTimedCondition();
    // Add implicit constraints
    std::for_each(candidates.begin(), candidates.end(), [&] (auto &candidate) {
      if (!candidate.empty()) {
        candidate.addImplicitConstraints(leftRightJuxtaposition);
      }
    });
    std::vector<JuxtaposedZoneSet> leftJuxtapositions, rightJuxtapositions;
    leftJuxtapositions.reserve(leftRow.size());
    rightJuxtapositions.reserve(leftRow.size());
    for (std::size_t i = 0; i < leftRow.size(); ++i) {
      leftJuxtapositions.emplace_back(leftRow.at(i), rightConcatenations.at(i), suffixes.at(i).wordSize());
      rightJuxtapositions.emplace_back(leftConcatenations.at(i), rightRow.at(i), suffixes.at(i).wordSize());
    }
    auto it = std::find_if(candidates.begin(), candidates.end(), [&](const auto &candidate) {
      return equivalence(leftRightJuxtaposition, leftJuxtapositions, rightJuxtapositions, candidate);
    });
    if (it == candidates.end()) {
      // We add other equations in this case
      while (!candidates.empty()) {
        const auto candidate = candidates.front();
        candidates.pop_front();
        for (const auto currentV1: constrainedV1) {
          for (const auto currentV2: v1Edges.at(currentV1)) {
            auto newCandidate = candidate;
            const auto edge = std::make_pair(currentV1, currentV2);
            auto insertIt = std::lower_bound(newCandidate.begin(), newCandidate.end(), edge);
            if (insertIt == newCandidate.end() || *insertIt != edge) {
              newCandidate.insert(insertIt, edge);
              if (equivalence(left, leftRow, right, rightRow, suffixes, newCandidate)) {
                return std::make_optional(newCandidate);
              } else {
                candidates.emplace_back(std::move(newCandidate));
              }
            }
          }
        }
      }
      return std::nullopt;
    } else {
      return std::make_optional(*it);
    }
  }
}
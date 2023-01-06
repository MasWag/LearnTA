/**
 * @author Masaki Waga
 * @date 2022/12/29.
 */

#pragma once

#include <utility>
#include <vector>
#include <unordered_set>

#include "common_types.hh"
#include "timed_automaton.hh"
#include "forward_regional_elementary_language.hh"

namespace learnta {

  /*!
   * @brief Elementary language with its neighbor conditions
   *
   * This is mainly used to handle imprecise clocks in the DTA construction.
   *
   * @sa ObservationTable
   * @invariant std::all_of(neighbors.begin(), neighbors.end(), [&] (const auto &neighbor) {return neighbor.getWord() == original.getWord();})
   * @invariant std::all_of(neighbors.begin(), neighbors.end(), [&] (const auto &neighbor) {return neighbor.getTimedCondition().size() == original.getTimedCondition().size();})
   * @invariant clockSize == original.getTimedCondition().size()
   * @invariant std::all_of(neighbors.begin(), neighbors.end(), [&] (const auto &neighbor) {return neighbor.getTimedCondition().size() == clockSize;})
   */
  class NeighborConditions {
  private:
    // The original elementary language
    ForwardRegionalElementaryLanguage original;
    // The precise clock variables
    std::unordered_set<ClockVariables> preciseClocks;
    // The neighbor elementary languages due to imprecise clocks
    std::vector<ForwardRegionalElementaryLanguage> neighbors;
    std::size_t clockSize;

    void assertInvariants() const {
      assert(std::all_of(neighbors.begin(), neighbors.end(), [&](const auto &neighbor) {
        return neighbor.getWord() == original.getWord();
      }));
      assert(clockSize == original.getTimedCondition().size());
      assert(std::all_of(neighbors.begin(), neighbors.end(), [&](const auto &neighbor) {
        return neighbor.getTimedCondition().size() == clockSize;
      }));
      assert(std::all_of(neighbors.begin(), neighbors.end(), [&](const auto &neighbor) {
        return std::all_of(preciseClocks.begin(), preciseClocks.end(), [&](const auto &preciseClock) {
          return neighbor.getTimedCondition().getLowerBound(preciseClock, clockSize - 1) ==
                 original.getTimedCondition().getLowerBound(preciseClock, clockSize - 1);
        });
      }));
      assert(std::all_of(neighbors.begin(), neighbors.end(), [&](const auto &neighbor) {
        return std::all_of(preciseClocks.begin(), preciseClocks.end(), [&](const auto &preciseClock) {
          return neighbor.getTimedCondition().getUpperBound(preciseClock, clockSize - 1) ==
                 original.getTimedCondition().getUpperBound(preciseClock, clockSize - 1);
        });
      }));
    }

    NeighborConditions(ForwardRegionalElementaryLanguage &&original,
                       std::vector<ForwardRegionalElementaryLanguage> &&neighbors,
                       const std::unordered_set<ClockVariables> &preciseClocks,
                       size_t clockSize) : original(original), preciseClocks(preciseClocks),
                                           neighbors(neighbors), clockSize(clockSize) {
      assertInvariants();
    }

    NeighborConditions(ForwardRegionalElementaryLanguage &&original,
                       std::vector<ForwardRegionalElementaryLanguage> &&neighbors,
                       std::unordered_set<ClockVariables> &&preciseClocks,
                       size_t clockSize) : original(original), preciseClocks(preciseClocks),
                                           neighbors(neighbors), clockSize(clockSize) {
      assertInvariants();
    }

  public:
    static auto makeNeighbors(const ForwardRegionalElementaryLanguage &original,
                              const std::unordered_set<ClockVariables> &preciseClocks) {
      const auto clockSize = original.getTimedCondition().size();
      std::vector<ForwardRegionalElementaryLanguage> neighbors;

      // Construct the neighbors
      auto neighborsCondition = TimedCondition(Zone::top(clockSize + 1));
      for (std::size_t i = 0; i < clockSize; ++i) {
        // The constraints of the form x \in I is used
        neighborsCondition.restrictLowerBound(i, clockSize - 1,
                                              original.getTimedCondition().getLowerBound(i, clockSize - 1));
        neighborsCondition.restrictUpperBound(i, clockSize - 1,
                                              original.getTimedCondition().getUpperBound(i, clockSize - 1));
        for (std::size_t j = i + 1; j < clockSize; ++j) {
          if ((preciseClocks.find(i) == preciseClocks.end()) ==
              (preciseClocks.find(j) == preciseClocks.end())) {
            // The constraints of the form x - y \in I is used if both are precise or imprecise
            neighborsCondition.restrictLowerBound(i, j - 1, original.getTimedCondition().getLowerBound(i, j - 1));
            neighborsCondition.restrictUpperBound(i, j - 1, original.getTimedCondition().getUpperBound(i, j - 1));
          } else {
            neighborsCondition.restrictLowerBound(i, j - 1, Bounds{0, true});
          }
        }
      }
      const auto simpleNeighborsCondition = neighborsCondition.enumerate();
      neighbors.reserve(simpleNeighborsCondition.size());
      std::transform(simpleNeighborsCondition.begin(), simpleNeighborsCondition.end(), std::back_inserter(neighbors),
                     [&](const auto &neighborCondition) {
                       return ForwardRegionalElementaryLanguage::fromTimedWord(
                               ElementaryLanguage{original.getWord(), neighborCondition}.sample());
                     });

      return neighbors;
    }

    NeighborConditions(const NeighborConditions& conditions) = default;
    NeighborConditions(NeighborConditions&& conditions) = default;
    NeighborConditions& operator=(const NeighborConditions& conditions) = default;
    NeighborConditions& operator=(NeighborConditions&& conditions) = default;
    NeighborConditions(const ForwardRegionalElementaryLanguage &original,
                       std::unordered_set<ClockVariables> preciseClocks) : original(original),
                                                                           preciseClocks(std::move(preciseClocks)),
                                                                           neighbors(makeNeighbors(this->original,
                                                                                                   this->preciseClocks)),
                                                                           clockSize(
                                                                                   this->original.getTimedCondition().size()) {
      assertInvariants();
    }

    NeighborConditions(const ForwardRegionalElementaryLanguage &original,
                       const std::vector<ClockVariables> &preciseClocks) : original(original),
                                                                           preciseClocks(
                                                                                   std::unordered_set<ClockVariables>(
                                                                                           preciseClocks.begin(),
                                                                                           preciseClocks.end())),
                                                                           neighbors(makeNeighbors(this->original,
                                                                                                   this->preciseClocks)),
                                                                           clockSize(
                                                                                   this->original.getTimedCondition().size()) {
      assertInvariants();
    }

    /*!
     * @brief Return the guard of the original elementary language.
     *
     * This is intended to be used to match with a transition
     */
    [[nodiscard]] auto toOriginalGuard() const {
      assertInvariants();
      return this->original.getTimedCondition().toGuard();
    }

    /*!
     * @brief Return if the given guard matches with this elementary language
     */
    [[nodiscard]] bool match(const std::vector<Constraint> &guard) const {
      assertInvariants();
      return isWeaker(guard, this->toOriginalGuard());
    }

    [[nodiscard]] size_t getClockSize() const {
      return clockSize;
    }

    /*!
     * @brief Returns if the relaxed guard is precise
     */
    [[nodiscard]] bool precise() const {
      return this->neighbors.size() == 1;
    }

    /*!
     * @brief Return if the given transition matches with this elementary language
     */
    [[nodiscard]] bool match(const TATransition &transition) const {
      assertInvariants();
      return this->match(transition.guard);
    }

    /*!
     * @brief Return the guard of the original elementary language and its neighbors.
     */
    [[nodiscard]] auto toRelaxedGuard() const {
      assertInvariants();
      std::vector<std::vector<Constraint>> guards;
      guards.reserve(this->neighbors.size());
      std::transform(this->neighbors.begin(), this->neighbors.end(), std::back_inserter(guards),
                     [&](const auto &neighbor) {
                       return neighbor.getTimedCondition().toGuard();
                     });

      return unionHull(guards);
    }

    [[nodiscard]] NeighborConditions successor(const Alphabet action) const {
      // The neighbor elementary languages due to imprecise clocks
      std::vector<ForwardRegionalElementaryLanguage> newNeighbors;
      newNeighbors.reserve(neighbors.size());
      std::transform(neighbors.begin(), neighbors.end(), std::back_inserter(newNeighbors), [&](const auto &neighbor) {
        return neighbor.successor(action);
      });
      auto newPreciseClocks = preciseClocks;
      newPreciseClocks.insert(clockSize);
      return NeighborConditions{original.successor(action), std::move(newNeighbors),
                                newPreciseClocks, clockSize + 1};
    }

    [[nodiscard]] NeighborConditions successor() const {
      auto originalSuccessor = original.successor();
      // The neighbor elementary languages due to imprecise clocks
      std::vector<ForwardRegionalElementaryLanguage> newNeighbors;
      newNeighbors.reserve(neighbors.size());
      for (auto neighbor: neighbors) {
        neighbor = neighbor.successor();
        while (std::all_of(preciseClocks.begin(), preciseClocks.end(), [&](const auto &preciseClock) {
          return neighbor.getTimedCondition().getUpperBound(preciseClock, neighbor.getTimedCondition().size() - 1) <=
                 originalSuccessor.getTimedCondition().getUpperBound(preciseClock,
                                                                     originalSuccessor.getTimedCondition().size() - 1);
        })) {
          if (std::all_of(preciseClocks.begin(), preciseClocks.end(), [&](const auto &preciseClock) {
            return neighbor.getTimedCondition().getLowerBound(preciseClock, neighbor.getTimedCondition().size() - 1) ==
                   originalSuccessor.getTimedCondition().getLowerBound(preciseClock,
                                                                       originalSuccessor.getTimedCondition().size() -
                                                                       1) &&
                   neighbor.getTimedCondition().getUpperBound(preciseClock, neighbor.getTimedCondition().size() - 1) ==
                   originalSuccessor.getTimedCondition().getUpperBound(preciseClock,
                                                                       originalSuccessor.getTimedCondition().size() -
                                                                       1);
          })) {
            newNeighbors.push_back(neighbor);
          }
          neighbor = neighbor.successor();
        }
      }

      std::sort(newNeighbors.begin(), newNeighbors.end(), [] (const auto& left, const auto& right) {
        return left.hash_value() < right.hash_value();
      });
      newNeighbors.erase(std::unique(newNeighbors.begin(), newNeighbors.end()), newNeighbors.end());
      return NeighborConditions{std::move(originalSuccessor), std::move(newNeighbors),
                                preciseClocks, clockSize};
    }

    void successorAssign() {
      original.successorAssign();
      // The neighbor elementary languages due to imprecise clocks
      std::vector<ForwardRegionalElementaryLanguage> newNeighbors;
      newNeighbors.reserve(neighbors.size());
      for (auto neighbor: neighbors) {
        neighbor.successorAssign();
        while (std::all_of(preciseClocks.begin(), preciseClocks.end(), [&](const auto &preciseClock) {
          return neighbor.getTimedCondition().getUpperBound(preciseClock, neighbor.getTimedCondition().size() - 1) <=
          original.getTimedCondition().getUpperBound(preciseClock, original.getTimedCondition().size() - 1);
        })) {
          if (std::all_of(preciseClocks.begin(), preciseClocks.end(), [&](const auto &preciseClock) {
            return neighbor.getTimedCondition().getLowerBound(preciseClock, neighbor.getTimedCondition().size() - 1) ==
            original.getTimedCondition().getLowerBound(preciseClock, original.getTimedCondition().size() - 1) &&
            neighbor.getTimedCondition().getUpperBound(preciseClock, neighbor.getTimedCondition().size() - 1) ==
            original.getTimedCondition().getUpperBound(preciseClock, original.getTimedCondition().size() - 1);
          })) {
            newNeighbors.push_back(neighbor);
          }
          neighbor.successorAssign();
        }
      }
      std::sort(newNeighbors.begin(), newNeighbors.end(), [] (const auto& left, const auto& right) {
        return left.hash_value() < right.hash_value();
      });
      newNeighbors.erase(std::unique(newNeighbors.begin(), newNeighbors.end()), newNeighbors.end());
      neighbors = std::move(newNeighbors);
    }
  };
}
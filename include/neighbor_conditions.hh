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
#include "external_transition_maker.hh"

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

    /*!
     * @brief Add implicitly precise clocks to this->preciseClocks
     *
     * A clock variable x is implicitly precise if we have c <= x <= c in original.
     */
    void addImplicitPreciseClocks() {
      BOOST_LOG_TRIVIAL(debug) << "explicit precise clocks: ";
      for (const auto &preciseClock: preciseClocks) {
        BOOST_LOG_TRIVIAL(debug) << "x" << static_cast<int>(preciseClock);
      }
      for (std::size_t i = 0; i < clockSize; ++i) {
        // We skip explicitly precise clocks
        if (this->preciseClocks.find(i) != this->preciseClocks.end()) {
          continue;
        }
        const auto lowerBound = this->original.getTimedCondition().getLowerBound(i, clockSize - 1);
        const auto upperBound = this->original.getTimedCondition().getUpperBound(i, clockSize - 1);
        if (isPoint(upperBound, lowerBound)) {
          assert(std::all_of(this->neighbors.begin(), this->neighbors.end(), [&] (const auto& neighbor) {
            return neighbor.getTimedCondition().getLowerBound(i, clockSize - 1) == lowerBound && neighbor.getTimedCondition().getUpperBound(i, clockSize - 1) == upperBound;
          }));
          this->preciseClocks.insert(i);
        }
      }
      BOOST_LOG_TRIVIAL(debug) << "appended precise clocks: ";
      for (const auto &preciseClock: preciseClocks) {
        BOOST_LOG_TRIVIAL(debug) << "x" << static_cast<int>(preciseClock);
      }
    }

    [[nodiscard]] auto updateNeighborsWithContinuousSuccessors(const ForwardRegionalElementaryLanguage &originalSuccessor) const {
      BOOST_LOG_TRIVIAL(debug) << "originalSuccessor: " << originalSuccessor;
      // The neighbor elementary languages due to imprecise clocks
      std::vector<ForwardRegionalElementaryLanguage> newNeighbors;
      newNeighbors.reserve(neighbors.size());
      for (auto neighbor: neighbors) {
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

      return newNeighbors;
    }

    /*!
     * @brief Make precise clocks after applying a reset
     */
    static auto preciseClocksAfterReset(const std::unordered_set<ClockVariables>& preciseClocks,
                                        const TATransition &transition) {
      std::unordered_set<ClockVariables> newPreciseClocks;
      const auto targetClockSize = computeTargetClockSize(transition);
      for (const auto &[targetVariable, assignedValue]: transition.resetVars) {
        if (targetVariable >= targetClockSize) {
          continue;
        }
        if (assignedValue.index() == 1 &&
            preciseClocks.find(std::get<ClockVariables>(assignedValue)) != preciseClocks.end()) {
          // targetVariable is precise if its value is updated to a precise variable
          newPreciseClocks.insert(targetVariable);
        } else if (assignedValue.index() == 0 &&
                   std::get<double>(assignedValue) == std::floor(std::get<double>(assignedValue))) {
          // targetVariable is precise if its value is updated to an integer
          newPreciseClocks.insert(targetVariable);
        }
      }
      for (const auto &preciseClock: preciseClocks) {
        // Check if the precise clock is in the range and not updated
        if (preciseClock >= targetClockSize || newPreciseClocks.find(preciseClock) != newPreciseClocks.end()) {
          continue;
        }
        auto it = std::find_if(transition.resetVars.begin(), transition.resetVars.end(), [&] (const auto &reset) {
          return reset.first == preciseClock;
        });
        if (it == transition.resetVars.end()) {
          newPreciseClocks.insert(preciseClock);
        }
      }

      assert(newPreciseClocks.size() <= targetClockSize);
      return newPreciseClocks;
    }

  public:
    /*!
     * @brief Make precise clocks after applying a reset
     */
    [[nodiscard]] auto preciseClocksAfterReset(const TATransition &transition) const {
      return NeighborConditions::preciseClocksAfterReset(this->preciseClocks, transition);
    }

    /*!
     * @brief Reconstruct the neighbor conditions with new precise clocks
     */
    [[nodiscard]] NeighborConditions reconstruct(std::unordered_set<ClockVariables> currentPreciseClocks) const {
      // Restrict the range of precise clocks
      for (auto it = currentPreciseClocks.begin(); it != currentPreciseClocks.end();) {
        if (*it > this->original.wordSize()) {
          it = currentPreciseClocks.erase(it);
        } else {
          ++it;
        }
      }
      return NeighborConditions{this->original, std::move(currentPreciseClocks)};
    }

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
    NeighborConditions(ForwardRegionalElementaryLanguage original,
                       std::unordered_set<ClockVariables> preciseClocks) : original(std::move(original)),
                                                                           preciseClocks(std::move(preciseClocks)),
                                                                           neighbors(makeNeighbors(this->original,
                                                                                                   this->preciseClocks)),
                                                                           clockSize(
                                                                                   this->original.getTimedCondition().size()) {
      addImplicitPreciseClocks();
      if (!this->preciseClocks.empty()) {
        this->neighbors = updateNeighborsWithContinuousSuccessors(this->original);
      }
      assertInvariants();
    }

    NeighborConditions(ForwardRegionalElementaryLanguage original,
                       const std::vector<ClockVariables> &preciseClocks) : original(std::move(original)),
                                                                           preciseClocks(
                                                                                   std::unordered_set<ClockVariables>(
                                                                                           preciseClocks.begin(),
                                                                                           preciseClocks.end())),
                                                                           neighbors(makeNeighbors(this->original,
                                                                                                   this->preciseClocks)),
                                                                           clockSize(
                                                                                   this->original.getTimedCondition().size()) {
      addImplicitPreciseClocks();
      if (!this->preciseClocks.empty()) {
        this->neighbors = updateNeighborsWithContinuousSuccessors(this->original);
      }
      assertInvariants();
    }

#if 0
    /*!
     * @brief Construct the neighbor conditions after an external transition
     *
     * @param resets The resets of the external transition
     * @param targetClockSize The clock size at the target state
     */
    [[nodiscard]] NeighborConditions makeAfterExternalTransition(const TATransition::Resets &resets,
                                                                 const std::size_t targetClockSize) const {
      // make words
      std::string newWord = this->original.getWord();
      newWord.resize(targetClockSize - 1, this->original.getWord().back());
      // make precise clocks
      auto newPreciseClocks = preciseClocksAfterReset(this->preciseClocks, resets);
      for (auto it = newPreciseClocks.begin(); it != newPreciseClocks.end();) {
        if (*it >= targetClockSize) {
          it = newPreciseClocks.erase(it);
        } else {
          ++it;
        }
      }
      // make original
      auto newOriginal = this->original.applyResets(newWord, resets, targetClockSize);
      // make neighbors
      std::vector<ForwardRegionalElementaryLanguage> newNeighbors;
      newNeighbors.reserve(this->neighbors.size());
      for (const ForwardRegionalElementaryLanguage &neighbor: this->neighbors) {
        const auto simpleConditionsAfterReset = neighbor.getTimedCondition().applyResets(resets,
                                                                                         targetClockSize).enumerate();
        std::transform(simpleConditionsAfterReset.begin(), simpleConditionsAfterReset.end(),
                       std::back_inserter(newNeighbors),
                       [&](const auto &simpleConditionAfterReset) {
                         return ForwardRegionalElementaryLanguage::fromTimedWord(
                                 ElementaryLanguage{newWord, simpleConditionAfterReset}.sample());
                       });
      }
      // Construct the resulting neighbor conditions
      NeighborConditions result = NeighborConditions{std::move(newOriginal),
                                                     std::move(newNeighbors),
                                                     std::move(newPreciseClocks),
                                                     targetClockSize};
      // Update the neighbors to include successors if the precise clocks match
      result.neighbors = result.updateNeighborsWithContinuousSuccessors(result.original);

      return result;
    }
#endif

    /*!
     * @brief Construct the neighbor conditions after a transition
     */
    [[nodiscard]] NeighborConditions makeAfterTransition(const Alphabet action, const TATransition &transition) const {
      return NeighborConditions{this->constructOriginalAfterTransition(action, transition),
                                this->preciseClocksAfterReset(transition)};
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
      auto newNeighbors = updateNeighborsWithContinuousSuccessors(originalSuccessor);

      return NeighborConditions{std::move(originalSuccessor), std::move(newNeighbors), preciseClocks, clockSize};
    }

    void successorAssign() {
      original.successorAssign();
      neighbors = updateNeighborsWithContinuousSuccessors(original);
    }

    /*!
     * @brief Returns the list of imprecise clocks
     *
     * @post The result is sorted
     */
     [[nodiscard]] std::vector<ClockVariables> impreciseClocks() const {
      std::vector<ClockVariables> impreciseClockVec;
      impreciseClockVec.reserve(this->clockSize);
      for (std::size_t clock = 0; clock < this->clockSize; ++clock) {
        if (this->preciseClocks.find(clock) == this->preciseClocks.end()) {
          impreciseClockVec.push_back(clock);
        }
      }

      return impreciseClockVec;
    }

    /*!
     * @brief Construct the reset to embed to the target condition
     */
    [[nodiscard]] std::vector<double> toOriginalValuation() const {
      return ExternalTransitionMaker::toValuation(this->original.getTimedCondition());
    }

    /*!
     * @brief Construct the reset to embed to the target condition
     */
    [[nodiscard]] std::vector<double> toOriginalValuation(const std::size_t minSize) const {
      auto valuation = ExternalTransitionMaker::toValuation(this->original.getTimedCondition());
      if (valuation.size() < minSize) {
        valuation.resize(minSize);
      }

      return valuation;
    }

    /*!
     * @brief Return the neighbor conditions after applying the given resets
     */
    [[nodiscard]] NeighborConditions applyResets(const TATransition::Resets &resets) const {
      auto newNeighborConditions = *this;
      newNeighborConditions.original = newNeighborConditions.original.applyResets(resets);
      for (auto &neighbor: newNeighborConditions.neighbors) {
        neighbor = neighbor.applyResets(resets);
      }
      for (const auto &[updatedVariable, assignedValue]: resets) {
        if (assignedValue.index() == 0) {
          newNeighborConditions.preciseClocks.insert(updatedVariable);
        } else {
          // This case is not supported
          BOOST_LOG_TRIVIAL(error) << "Unimplemented case";
          abort();
        }
      }

      return newNeighborConditions;
    }

    /*!
     * @brief Check if the transition from the current neighbor is internal
     */
    [[nodiscard]] bool isInternal(const TATransition &transition) const {
      return transition.resetVars.size() == 1 &&
             transition.resetVars.front().first == this->getClockSize() &&
             transition.resetVars.front().second.index() == 0 &&
             std::get<double>(transition.resetVars.front().second) == 0.0;
    }

    [[nodiscard]] ForwardRegionalElementaryLanguage constructOriginalAfterTransition(const Alphabet action,
                                                                                     const TATransition &transition) const {
      if (isInternal(transition)) {
        return this->original.successor(action);
      } else {
        // make words
        std::string newWord = this->original.getWord();
        const auto targetClockSize = computeTargetClockSize(transition);
        assert(targetClockSize > 0);
        newWord.resize(targetClockSize - 1, this->original.getWord().back());

        return this->original.applyResets(newWord, transition.resetVars, targetClockSize);
      }
    }

    [[nodiscard]] static std::size_t computeTargetClockSize(const TATransition &transition) {
      std::size_t targetClockSize = 0;
      for (const auto &[action, transitions]: transition.target->next) {
        for (const auto &cTransition: transitions) {
          std::for_each(cTransition.guard.begin(), cTransition.guard.end(), [&](const Constraint &constraint) {
            targetClockSize = std::max(targetClockSize, static_cast<std::size_t>(constraint.x + 1));
          });
        }
      }

      return targetClockSize;
    }

    std::ostream &print(std::ostream &os) const {
      os << this->original << " {";
      bool initial = true;
      for (const auto &preciseClock: preciseClocks) {
        if (!initial) {
          os << ", ";
        }
        os << "x" << static_cast<int>(preciseClock);
        initial = false;
      }
      os << "} {";
      for (const auto &neighbor: neighbors) {
        os << "\n" << neighbor;
      }
      os << "\n}";

      return os;
    }

    bool operator==(const NeighborConditions &conditions) const {
      return original == conditions.original &&
             preciseClocks == conditions.preciseClocks &&
             neighbors == conditions.neighbors &&
             clockSize == conditions.clockSize;
    }

    bool operator!=(const NeighborConditions &conditions) const {
      return !(conditions == *this);
    }

    [[nodiscard]] std::size_t hash_value() const {
      return boost::hash_value(std::make_tuple(original,
                                               std::vector<ClockVariables>{preciseClocks.begin(), preciseClocks.end()},
                                               neighbors, clockSize));
    }
  };

  static inline std::ostream &operator<<(std::ostream &os, const learnta::NeighborConditions &conditions) {
    return conditions.print(os);
  }

  static inline std::size_t hash_value(const NeighborConditions &conditions) {
    return conditions.hash_value();
  }
}
/**
 * @author Masaki Waga
 * @date 2022/03/07.
 */

#pragma once

#include <utility>
#include <vector>

#include "sul.hh"
#include "timed_automaton.hh"

namespace learnta {
  /*!
   * @brief Class to execute a timed automaton
   *
   * @note We assume that the given timed automaton is \em deterministic.
   * @note We support unobservable transitions, but we assume that there is no loop of unobservable transitions
   * @invariant this->clockValuation.size() == this->automaton.maxConstraints.size();
   */
  class TimedAutomatonRunner : public SUL {
  private:
    TimedAutomaton automaton;
    TAState *state;
    std::vector<double> clockValuation;
    std::size_t numQueries;
    const bool isEmpty = false;
  public:
    explicit TimedAutomatonRunner(TimedAutomaton automaton) : automaton(std::move(automaton)),
                                                              isEmpty(this->automaton.states.empty()) {
      if (!isEmpty) {
        assert(this->automaton.initialStates.size() == 1);
        this->state = this->automaton.initialStates.at(0).get();
        this->clockValuation.resize(this->automaton.maxConstraints.size());
      }
      numQueries = 0;
    }

    //! @brief Reset the configuration
    void pre() override {
      if (!isEmpty) {
        // We assume that the given timed automaton is deterministic
        assert(this->clockValuation.size() == this->automaton.maxConstraints.size());
        this->state = this->automaton.initialStates.at(0).get();
        std::fill(this->clockValuation.begin(), this->clockValuation.end(), 0);
      }
      numQueries++;
    }

    void post() override {
      if (!isEmpty) {
        assert(this->clockValuation.size() == this->automaton.maxConstraints.size());
      }
    }

    /*!
     * @brief Apply the given reset to the clock valuation
     */
    void applyReset(const TATransition::Resets &resets) {
      const auto oldValuation = this->clockValuation;
      for (const auto &[resetVariable, targetVariable]: resets) {
        if (targetVariable.index() == 1) {
          this->clockValuation.at(resetVariable) = oldValuation.at(std::get<ClockVariables>(targetVariable));
        } else {
          this->clockValuation.at(resetVariable) = std::get<double>(targetVariable);
        }
      }
    }

    bool step(char action) override {
      if (isEmpty) {
        return false;
      }
      assert(this->clockValuation.size() == this->automaton.maxConstraints.size());
      // Skip the operation if we are at the sink state
      if (this->state == nullptr) {
        return false;
      }
      for (const TATransition &transition: this->state->next[action]) {
        // Check if the guard is satisfied
        if (std::all_of(transition.guard.begin(), transition.guard.end(), [&](const Constraint &guard) {
          return guard.satisfy(this->clockValuation.at(guard.x));
        })) {
          // Reset the clock variables
          this->applyReset(transition.resetVars);
          this->state = transition.target;

          return this->state->isMatch;
        }
      }

      // If there is no outgoing transition, we go to the sink state
      state = nullptr;
      return false;
    }

    bool step(double duration) override {
      if (isEmpty) {
        return false;
      }
      assert(this->clockValuation.size() == this->automaton.maxConstraints.size());
      // Skip the operation if we are at the sink state
      if (this->state == nullptr) {
        return false;
      }
      const auto unobservableIt = this->state->next.find(UNOBSERVABLE);
      if (unobservableIt != this->state->next.end() && !unobservableIt->second.empty()) {
        // If there are unobservable transitions, we choose the minimum duration to satisfy it
        double minDuration;
        auto candidateTransition = unobservableIt->second.end();
        if (unobservableIt->second.size() == 1) {
          minDuration = lowerBoundDurationToSatisfy(unobservableIt->second.front().guard, this->clockValuation);
          candidateTransition = unobservableIt->second.begin();
        } else {
          candidateTransition = std::min_element(unobservableIt->second.begin(), unobservableIt->second.end(),
                                                 [&](const auto &a, const auto &b) {
                                                   return lowerBoundDurationToSatisfy(a.guard, this->clockValuation) <
                                                          lowerBoundDurationToSatisfy(b.guard, this->clockValuation);
                                                 });
          minDuration = lowerBoundDurationToSatisfy(candidateTransition->guard, this->clockValuation);
        }
        if (std::isfinite(minDuration) && minDuration >= 0 && duration >= minDuration) {
          // An unobservable transition is available
          std::transform(this->clockValuation.begin(), this->clockValuation.end(), this->clockValuation.begin(),
                         [&](double value) {
                           return value + minDuration;
                         });
          // Reset the clock variables
          // assert the validity of the candidate transition
          assert(candidateTransition != unobservableIt->second.end());
          this->applyReset(candidateTransition->resetVars);
          this->state = candidateTransition->target;
          return this->step(duration - minDuration);
        } else {
          BOOST_LOG_TRIVIAL(debug) << "Unobservable transitions are skipped";
          BOOST_LOG_TRIVIAL(debug) << "std::isfinite(minDuration): " << std::isfinite(minDuration);
          BOOST_LOG_TRIVIAL(debug) << "minDuration: " << minDuration;
          BOOST_LOG_TRIVIAL(debug) << "duration: " << duration;
        }
      }
      // No unobservable transition is available
      std::transform(this->clockValuation.begin(), this->clockValuation.end(), this->clockValuation.begin(),
                     [&](double value) {
                       return value + duration;
                     });

      return this->state->isMatch;
    }

    [[nodiscard]] std::size_t count() const override {
      return numQueries;
    }
  };
}

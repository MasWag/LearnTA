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
   * @note We assume that the given timed automaton is \em deterministic and with <em>no unobservable transitions</em>.
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
          const auto oldValuation = this->clockValuation;
          for (const auto &[resetVariable, targetVariable]: transition.resetVars) {
            if (targetVariable) {
              this->clockValuation.at(resetVariable) = oldValuation.at(*targetVariable);
            } else {
              this->clockValuation.at(resetVariable) = 0;
            }
          }
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
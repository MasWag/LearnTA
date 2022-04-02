/**
 * @author Masaki Waga
 * @date 2022/03/15.
 */

#pragma once

#include <vector>
#include <string>
#include <list>

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

#include "timed_word.hh"
#include "zone.hh"
#include "timed_automaton.hh"
#include "zone_automaton_state.hh"

namespace learnta {
  /*!
   * @brief Run of a zone automaton
   *
   * @sa ZoneAutomaton
   */
  class SymbolicRun {
  private:
    std::vector<std::shared_ptr<ZAState>> states;
    std::vector<learnta::TATransition> edges;
    std::string word;
  public:
    explicit SymbolicRun(const std::shared_ptr<ZAState> &initialState) : states({initialState}), edges() {}

    void push_back(const learnta::TATransition &transition, char action, const std::shared_ptr<ZAState> &state) {
      this->states.push_back(state);
      word.push_back(action);
      this->edges.push_back(transition);
    }

    [[nodiscard]] std::shared_ptr<ZAState> stateAt(int i) const {
      return this->states.at(i);
    }

    [[nodiscard]] learnta::TATransition edgeAt(int i) const {
      return this->edges.at(i);
    }

    [[nodiscard]] std::shared_ptr<ZAState> back() const {
      return this->states.back();
    }

    /*!
     * @breif Reconstruct a timed word from a symbolic run
     *
     * We use the reconstruction algorithm in [Andre+, NFM'22]
     * We note that our zone construction is state --time_elapse--> intermediate --discrete_jump--> next_state.
     */
    [[nodiscard]] TimedWord reconstructWord() const {
      // Sample the last concrete state;
      auto postZoneState = this->back();
      auto postZone = postZoneState->zone;
      auto postValuation = postZone.sample();

      const auto N = this->edges.size();
      std::list<double> durations;

      for (int i = N - 1; i >= 0; --i) {
        auto preZoneState = this->stateAt(i);
        auto preZone = preZoneState->zone;

        // Construct the zone just before jump
        auto zoneBeforeJump = Zone{postValuation, postZone.M};
        if (!zoneBeforeJump.isSatisfiableNoCanonize()) {
          BOOST_LOG_TRIVIAL(error) << "Failed to reconstruct word from a symbolic run\n" << *this;
        }
        assert(zoneBeforeJump.isSatisfiableNoCanonize());
        const auto transition = this->edgeAt(i);
        for (const auto &[resetVariable, updatedVariable]: transition.resetVars) {
          zoneBeforeJump.unconstrain(resetVariable);
        }
        assert(zoneBeforeJump.isSatisfiableNoCanonize());
        for (const auto guard: transition.guard) {
          zoneBeforeJump.tighten(guard);
          if (!zoneBeforeJump.isSatisfiableNoCanonize()) {
            BOOST_LOG_TRIVIAL(error) << "Failed to reconstruct word from a symbolic run\n" << *this;
            BOOST_LOG_TRIVIAL(error) << "The unsatisfiable zone\n" << zoneBeforeJump;
          }
          assert(zoneBeforeJump.isSatisfiableNoCanonize());
        }
        {
          auto tmpPreZone = preZone;
          tmpPreZone.elapse();
          zoneBeforeJump &= tmpPreZone;
          assert(zoneBeforeJump.isSatisfiableNoCanonize());
        }

        // Sample the valuation just before discrete jump
        assert(zoneBeforeJump.isSatisfiableNoCanonize());
        const auto valuationBeforeJump = zoneBeforeJump.sample();
        auto backwardPreZone = Zone{valuationBeforeJump, postZone.M};
        backwardPreZone.reverseElapse();
        const auto preValuation = (preZone && backwardPreZone).sample();
        durations.push_front(valuationBeforeJump.at(0) - preValuation.at(0));

        // Update the variables
        postZone = std::move(preZone);
        postValuation = preValuation;
      }
      // Return the timed word
      std::vector<double> durationsVector;
      durationsVector.resize(durations.size());
      std::move(durations.begin(), durations.end(), durationsVector.begin());
      durationsVector.push_back(0);

      return {word, durationsVector};
    }

    //! @brief Print the symbolic run
    static inline std::ostream &print(std::ostream &os, const learnta::SymbolicRun &run) {
      os << run.states.front()->zone;

      for (int i = 0; i < run.word.size(); ++i) {
        os << run.word.at(i) << "\n";
        for (const auto &guard: run.edges.at(i).guard) {
          os << guard << ", ";
        }
        for (const auto &[resetVar, updatedVarOpt]: run.edges.at(i).resetVars) {
          if (updatedVarOpt) {
            os << "x" << int(resetVar) << " := x" << int(*updatedVarOpt) << ", ";
          } else {
            os << "x" << int(resetVar) << " := 0, ";
          }
        }
        os << "\n" << run.states.at(i + 1)->zone;
      }

      return os;
    }
  };

  static inline std::ostream &operator<<(std::ostream &os, const learnta::SymbolicRun &run) {
    return learnta::SymbolicRun::print(os, run);
  }
}
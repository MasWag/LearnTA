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
    // A sequence of the visited zones without abstraction
    std::vector<Zone> tightZones;
    std::vector<learnta::TATransition> edges;
    std::string word;
  public:
    explicit SymbolicRun(const std::shared_ptr<ZAState> &initialState) : states({initialState}), edges() {
      tightZones = {this->states.front()->zone};
    }

    //! @brief Push a new edge and state in the end of the run
    void push_back(const learnta::TATransition &transition, char action, const std::shared_ptr<ZAState> &state) {
      assert(this->states.size() == this->tightZones.size());
      assert(this->states.size() - 1 == this->word.size());
      assert(this->word.size() == this->edges.size());
      this->states.push_back(state);
      word.push_back(action);
      this->edges.push_back(transition);
      tightZones.push_back(tightZones.back());
      tightZones.back().elapse();
      tightZones.back().tighten(this->edges.back().guard);
      tightZones.back().applyResets(this->edges.back().resetVars);
      tightZones.back().canonize();
    }

    //! @brief Returns the i-th state in the symbolic run
    [[nodiscard]] std::shared_ptr<ZAState> stateAt(int i) const {
      return this->states.at(i);
    }

    //! @brief Returns the i-th edge in the symbolic run
    [[nodiscard]] learnta::TATransition edgeAt(int i) const {
      return this->edges.at(i);
    }

    [[nodiscard]] learnta::Zone tightZoneAt(int i) const {
      return this->tightZones.at(i);
    }

    //! @brief Returns the last state in the symbolic run
    [[nodiscard]] std::shared_ptr<ZAState> back() const {
      return this->states.back();
    }

    /*!
     * @brief Reconstruct a timed word from a symbolic run
     *
     * We use the reconstruction algorithm in [Andre+, NFM'22].
     * Due to the state merging in the zone construction, the reconstruction may fail.
     * We note that our zone construction is state --time_elapse--> intermediate --discrete_jump--> next_state.
     */
    [[nodiscard]] std::optional<TimedWord> reconstructWord() const {
      // Sample the last concrete state;
      auto postZoneState = this->back();
      auto postZone = this->tightZones.back();
      if (!postZone.isSatisfiable()) {
        return std::nullopt;
      }
      auto postValuation = postZone.sample();

      const auto N = this->edges.size();
      std::list<double> durations;

      for (int i = N - 1; i >= 0; --i) {
        auto preZone = this->tightZoneAt(i);

        // Construct the zone just before jump
        auto zoneBeforeJump = Zone{postValuation, postZone.M};
        if (!zoneBeforeJump.isSatisfiableNoCanonize()) {
          BOOST_LOG_TRIVIAL(error) << "Failed to reconstruct word from a symbolic run\n" << *this;
          return std::nullopt;
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
            return std::nullopt;
          }
          assert(zoneBeforeJump.isSatisfiableNoCanonize());
        }
        {
          auto tmpPreZone = preZone;
          tmpPreZone.elapse();
          zoneBeforeJump &= tmpPreZone;
          if (!zoneBeforeJump.isSatisfiableNoCanonize()) {
            // The word reconstruction may fail due to state mering
            BOOST_LOG_TRIVIAL(debug) << "Failed to reconstruct word from a symbolic run\n" << *this;
            return std::nullopt;
          }
        }

        // Sample the valuation just before discrete jump
        assert(zoneBeforeJump.isSatisfiableNoCanonize());
        const auto valuationBeforeJump = zoneBeforeJump.sample();
        auto backwardPreZone = Zone{valuationBeforeJump, postZone.M};
        backwardPreZone.reverseElapse();
        const auto preValuation = (preZone && backwardPreZone).sample();
        if (preValuation.empty()) {
          durations.push_front(0);
        } else {
          durations.push_front(valuationBeforeJump.at(0) - preValuation.at(0));
        }

        // Update the variables
        postZone = std::move(preZone);
        postValuation = preValuation;
      }
      // Return the timed word
      std::vector<double> durationsVector;
      durationsVector.resize(durations.size());
      std::move(durations.begin(), durations.end(), durationsVector.begin());
      durationsVector.push_back(0);

      return std::make_optional(TimedWord{word, durationsVector});
    }

    //! @brief Print the symbolic run
    static inline std::ostream &print(std::ostream &os, const learnta::SymbolicRun &run) {
      os << run.tightZones.front();

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
        os << "\n" << run.tightZoneAt(i + 1);
      }

      return os;
    }
  };

  //! @brief Print the symbolic run
  static inline std::ostream &operator<<(std::ostream &os, const learnta::SymbolicRun &run) {
    return learnta::SymbolicRun::print(os, run);
  }
}

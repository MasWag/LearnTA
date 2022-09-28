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

namespace std {
  inline static std::ostream& operator<<(std::ostream& stream, const std::vector<double> &valuation) {
    stream << "{";
    for (std::size_t i = 0; i < valuation.size(); ++i) {
      if (i > 0) {
        stream << ", ";
      }
      stream << valuation.at(i);
    }
    stream << "}";
    return stream;
  }
}
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
      BOOST_LOG_TRIVIAL(trace) << "Started reconstructWord";
      // The zone after the jump (here, this is the last zone)
      auto postZone = this->tightZones.back();
      if (!postZone.isSatisfiable()) {
        return std::nullopt;
      }
      // Sample the last concrete state
      auto postValuation = postZone.sample();

      const auto N = this->edges.size();
      std::list<double> durations;

      for (int i = N - 1; i >= 0; --i) {
        BOOST_LOG_TRIVIAL(trace) << "postValuation: " << postValuation;
        // The zone just after the previous jump
        auto preZone = this->tightZoneAt(i);
        preZone.canonize();

        // Construct the zone just before jump
        auto zoneBeforeJump = Zone{postValuation, postZone.M};
        if (!zoneBeforeJump.isSatisfiableNoCanonize()) {
          BOOST_LOG_TRIVIAL(error) << "Failed to reconstruct word from a symbolic run\n" << *this;
          return std::nullopt;
        }
        const auto transition = this->edgeAt(i);
        // Handle the reset
        zoneBeforeJump.revertResets(transition.resetVars);
        assert(zoneBeforeJump.isSatisfiableNoCanonize());
        // Handle the guard
        for (const auto guard: transition.guard) {
          zoneBeforeJump.tighten(guard);
          if (!zoneBeforeJump.isSatisfiableNoCanonize()) {
            BOOST_LOG_TRIVIAL(error) << "Failed to reconstruct word from a symbolic run\n" << *this;
            BOOST_LOG_TRIVIAL(error) << "The unsatisfiable zone\n" << zoneBeforeJump;
            return std::nullopt;
          }
        }
        {
          // Use the precondition
          auto tmpZoneBeforeJump = zoneBeforeJump;
          tmpZoneBeforeJump.reverseElapse();
          tmpZoneBeforeJump &= preZone;
          if (!tmpZoneBeforeJump.isSatisfiable()) {
            // The word reconstruction may fail due to state mering
            BOOST_LOG_TRIVIAL(debug) << "Failed to reconstruct word from a symbolic run\n" << *this;
            return std::nullopt;
          }
          tmpZoneBeforeJump.elapse();
          zoneBeforeJump &= tmpZoneBeforeJump;
        }

        // Sample the valuation just before discrete jump
        assert(zoneBeforeJump.isSatisfiableNoCanonize());
        const auto valuationBeforeJump = zoneBeforeJump.sample();
        assert(std::all_of(transition.guard.begin(), transition.guard.end(), [&] (const auto &guard) {
          return guard.satisfy(valuationBeforeJump);
        }));
        auto backwardPreZone = Zone{valuationBeforeJump, postZone.M};
        backwardPreZone.reverseElapse();
        const auto preValuation = (preZone && backwardPreZone).sample();
        if (preValuation.empty()) {
          durations.push_front(0);
        } else {
          BOOST_LOG_TRIVIAL(trace) << "valuationBeforeJump: " << valuationBeforeJump;
          BOOST_LOG_TRIVIAL(trace) << "preValuation: " << preValuation;
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

      assert(validate(durationsVector));
      return std::make_optional(TimedWord{word, durationsVector});
    }

    /*!
     * @brief Check if the given list of durations is consistent with the symbolic run.
     */
    [[nodiscard]] bool validate(const std::vector<double> &durations) const {
      std::vector<double> valuation;
      valuation.resize(this->tightZones.front().getNumOfVar());
      for (std::size_t i = 0; i < this->edges.size(); ++i) {
        std::transform(valuation.begin(), valuation.end(), valuation.begin(), [&] (const auto &value) {
          return value + durations.at(i);
        });
        const auto transition = this->edges.at(i);
        if (std::all_of(transition.guard.begin(), transition.guard.end(), [&](const Constraint &guard) {
          return guard.satisfy(valuation);
        })) {
          // Reset the clock variables
          const auto oldValuation = valuation;
          for (const auto &[resetVariable, targetVariable]: transition.resetVars) {
            if (targetVariable.index() == 1) {
              valuation.at(resetVariable) = oldValuation.at(std::get<ClockVariables>(targetVariable));
            } else {
              valuation.at(resetVariable) = std::get<double>(targetVariable);
            }
          }
          BOOST_LOG_TRIVIAL(trace) << "valuation: " << valuation;
        } else {
          return false;
        }
      }

      return true;
    }

    //! @brief Print the symbolic run
    static inline std::ostream &print(std::ostream &os, const learnta::SymbolicRun &run) {
      os << run.tightZones.front();

      for (std::size_t i = 0; i < run.word.size(); ++i) {
        os << run.word.at(i) << "\n";
        os << run.edges.at(i).guard << run.edges.at(i).resetVars;
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

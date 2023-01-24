/**
 * @author Masaki Waga
 * @date 2023/01/24.
 */

#pragma once

#include <vector>

#include "common_types.hh"
#include "timed_automaton.hh"

/**
 * @brief Fixture of "Unbalanced loop" benchmark, which is inspired by the unbalanced TRE in [ACM'02].
 */
struct UnbalancedLoopFixture {
  const std::vector<learnta::Alphabet> alphabet = {'a'};
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  /*!
   * @brief The constructor
   *
   * @param states The number of the states
   * @param clocks The complexity level
   * @param scale The least timing constraint in the guards
   */
  explicit UnbalancedLoopFixture(const int states, const int clocks, const int scale = 1) {
    assert(states > 0);
    assert(clocks > 0);
    // Generate the states
    targetAutomaton.states.reserve(states);
    for (int i = 0; i < states; ++i) {
      targetAutomaton.states.push_back(std::make_shared<learnta::TAState>(true));
    }
    for (int i = 0; i < states; ++i) {
      targetAutomaton.states.at(i)->next['a'].emplace_back(targetAutomaton.states.at((i + 1) % states).get(),
                                                           learnta::TATransition::Resets{},
                                                           std::vector<learnta::Constraint>{});
    }
    for (learnta::ClockVariables clock = 0; clock < clocks; ++clock) {
      int length;
      for (length = 1; clock >= ((length + 1)  * length) / 2; ++length);
      const int init = clock - ((length - 1)  * length) / 2;
      for (int state = init; state <= states - length; state += length) {
        auto source = targetAutomaton.states.at((state + length - 1) % states);
        source->next.at('a').front().resetVars.emplace_back(clock, 0.0);
        source->next.at('a').front().guard.push_back(learnta::ConstraintMaker(clock) > length * scale);
        source->next.at('a').front().guard.push_back(learnta::ConstraintMaker(clock) < 2 * length * scale);
      }
      targetAutomaton.maxConstraints.push_back(scale * length);
    }
    targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));

    BOOST_LOG_TRIVIAL(info) << "target DTA before simplification\n" << targetAutomaton;

    // simplify the target DTA
    targetAutomaton.simplifyStrong();
    targetAutomaton.simplifyWithZones();

    // Construct the complement DTA
    complementTargetAutomaton = targetAutomaton.complement(this->alphabet);
    complementTargetAutomaton.simplifyStrong();
    complementTargetAutomaton.simplifyWithZones();
  }
};
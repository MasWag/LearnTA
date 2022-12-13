/**
 * @author Masaki Waga
 * @date 2022/03/29.
 */

#pragma once

#include <vector>

#include "common_types.hh"
#include "timed_automaton.hh"

/**
 * @brief Fixture of the Light DTA in [APT'20]
 *
 * - [APT'20]: Aichernig, Bernhard K., Andrea Pferscher, and Martin Tappler. "From passive to active: learning timed automata efficiently." NASA Formal Methods Symposium. Springer, Cham, 2020.
 */
struct LightAutomatonFixture {
  const std::vector<learnta::Alphabet> alphabet = {'p', 'r', 's', 'e', 't'};
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;
  const int scale;

  explicit LightAutomatonFixture(int scale = 5) : scale(scale) {
    // Generate the target DTA
    targetAutomaton.states.resize(6);
    for (int i = 0; i < 5; ++i) {
      targetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(true);
    }
    targetAutomaton.states.at(5) = std::make_shared<learnta::TAState>(false);

    // Transitions
    targetAutomaton.states.at(0)->next['p'].emplace_back();
    targetAutomaton.states.at(0)->next['p'].back().target = targetAutomaton.states.at(1).get();
    targetAutomaton.states.at(0)->next['p'].back().resetVars.emplace_back(0, 0.0);

    targetAutomaton.states.at(1)->next['r'].emplace_back();
    targetAutomaton.states.at(1)->next['r'].back().target = targetAutomaton.states.at(0).get();
    targetAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) < scale};
    targetAutomaton.states.at(1)->next['r'].emplace_back();
    targetAutomaton.states.at(1)->next['r'].back().target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= scale};

    targetAutomaton.states.at(1)->next['s'].emplace_back();
    targetAutomaton.states.at(1)->next['s'].back().target = targetAutomaton.states.at(2).get();
    targetAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(0) >= 2 * scale};
    targetAutomaton.states.at(1)->next['s'].emplace_back();
    targetAutomaton.states.at(1)->next['s'].back().target = targetAutomaton.states.at(5).get();
    targetAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(0) < 2 * scale};

    targetAutomaton.states.at(2)->next['r'].emplace_back();
    targetAutomaton.states.at(2)->next['r'].back().target = targetAutomaton.states.at(3).get();

    targetAutomaton.states.at(3)->next['e'].emplace_back();
    targetAutomaton.states.at(3)->next['e'].back().target = targetAutomaton.states.at(0).get();

    targetAutomaton.states.at(4)->next['t'].emplace_back();
    targetAutomaton.states.at(4)->next['t'].back().target = targetAutomaton.states.at(0).get();

    targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
    targetAutomaton.maxConstraints.resize(1);
    targetAutomaton.maxConstraints[0] = 2 * scale;

    // Generate the complement of the target DTA
    complementTargetAutomaton.states.resize(6);
    for (int i = 0; i < 5; ++i) {
      complementTargetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(false);
    }
    complementTargetAutomaton.states.at(5) = std::make_shared<learnta::TAState>(true);

    // Transitions
    complementTargetAutomaton.states.at(0)->next['p'].emplace_back();
    complementTargetAutomaton.states.at(0)->next['p'].back().target = complementTargetAutomaton.states.at(1).get();
    complementTargetAutomaton.states.at(0)->next['p'].back().resetVars.emplace_back(0, 0.0);

    complementTargetAutomaton.states.at(1)->next['r'].emplace_back();
    complementTargetAutomaton.states.at(1)->next['r'].back().target = complementTargetAutomaton.states.at(0).get();
    complementTargetAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) < scale};
    complementTargetAutomaton.states.at(1)->next['r'].emplace_back();
    complementTargetAutomaton.states.at(1)->next['r'].back().target = complementTargetAutomaton.states.at(4).get();
    complementTargetAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= scale};

    complementTargetAutomaton.states.at(1)->next['s'].emplace_back();
    complementTargetAutomaton.states.at(1)->next['s'].back().target = complementTargetAutomaton.states.at(2).get();
    complementTargetAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(0) >= 2 * scale};
    complementTargetAutomaton.states.at(1)->next['s'].emplace_back();
    complementTargetAutomaton.states.at(1)->next['s'].back().target = complementTargetAutomaton.states.at(5).get();
    complementTargetAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(0) < 2 * scale};

    complementTargetAutomaton.states.at(2)->next['r'].emplace_back();
    complementTargetAutomaton.states.at(2)->next['r'].back().target = complementTargetAutomaton.states.at(3).get();

    complementTargetAutomaton.states.at(3)->next['e'].emplace_back();
    complementTargetAutomaton.states.at(3)->next['e'].back().target = complementTargetAutomaton.states.at(0).get();

    complementTargetAutomaton.states.at(4)->next['t'].emplace_back();
    complementTargetAutomaton.states.at(4)->next['t'].back().target = complementTargetAutomaton.states.at(0).get();

    // If the transition is empty, we make a transition to the sink state
    for (auto& state: complementTargetAutomaton.states) {
      for (const auto& action: alphabet) {
        if (state->next.find(action) == state->next.end()) {
          state->next[action].emplace_back();
          state->next.at(action).back().target = complementTargetAutomaton.states.at(5).get();
        }
      }
    }

    complementTargetAutomaton.initialStates.push_back(complementTargetAutomaton.states.at(0));
    complementTargetAutomaton.maxConstraints.resize(1);
    complementTargetAutomaton.maxConstraints[0] = 2 * scale;
  }
};
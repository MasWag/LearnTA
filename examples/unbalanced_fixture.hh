/**
 * @author Masaki Waga
 * @date 2022/09/15.
 */

#pragma once

#include <vector>

#include "common_types.hh"
#include "timed_automaton.hh"

/**
 * @brief Fixture of "Unbalanced" benchmark, which is inspired by the unbalanced TRE in [ACM'02]
 */
struct UnbalancedFixture {
  const std::vector<learnta::Alphabet> alphabet = {'a', 'b', 'c'};
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  explicit UnbalancedFixture(const int scale = 1) {
    // Generate the target DTA
    targetAutomaton.states.resize(5);
    // The initial state
    targetAutomaton.states.at(0) = std::make_shared<learnta::TAState>(false);
    targetAutomaton.states.at(0)->next['a'].resize(1);
    targetAutomaton.states.at(0)->next['b'].resize(1);
    targetAutomaton.states.at(0)->next['c'].resize(1);
    // The state waiting for b
    targetAutomaton.states.at(1) = std::make_shared<learnta::TAState>(false);
    targetAutomaton.states.at(1)->next['a'].resize(1);
    targetAutomaton.states.at(1)->next['b'].resize(3);
    targetAutomaton.states.at(1)->next['c'].resize(1);
    // The state waiting for c
    targetAutomaton.states.at(2) = std::make_shared<learnta::TAState>(false);
    targetAutomaton.states.at(2)->next['a'].resize(1);
    targetAutomaton.states.at(2)->next['b'].resize(1);
    targetAutomaton.states.at(2)->next['c'].resize(3);
    // The accepting state
    targetAutomaton.states.at(3) = std::make_shared<learnta::TAState>(true);
    targetAutomaton.states.at(3)->next['a'].resize(1);
    targetAutomaton.states.at(3)->next['b'].resize(1);
    targetAutomaton.states.at(3)->next['c'].resize(1);
    // The sink state
    targetAutomaton.states.at(4) = std::make_shared<learnta::TAState>(false);
    targetAutomaton.states.at(4)->next['a'].resize(1);
    targetAutomaton.states.at(4)->next['b'].resize(1);
    targetAutomaton.states.at(4)->next['c'].resize(1);
    // Transitions from loc0
    targetAutomaton.states.at(0)->next['a'].at(0).target = targetAutomaton.states.at(1).get();
    targetAutomaton.states.at(0)->next['a'].at(0).resetVars = {{1, 0.0}};
    targetAutomaton.states.at(0)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(0)->next['c'].at(0).target = targetAutomaton.states.at(4).get();
    // Transitions from loc1
    targetAutomaton.states.at(1)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(1)->next['b'].at(0).target = targetAutomaton.states.at(2).get();
    targetAutomaton.states.at(1)->next['b'].at(0).guard = {learnta::ConstraintMaker(0) >= scale,
                                                           learnta::ConstraintMaker(0) <= scale};
    targetAutomaton.states.at(1)->next['b'].at(1).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(1)->next['b'].at(1).guard = {learnta::ConstraintMaker(0) > scale};
    targetAutomaton.states.at(1)->next['b'].at(2).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(1)->next['b'].at(2).guard = {learnta::ConstraintMaker(0) < scale};
    targetAutomaton.states.at(1)->next['c'].at(0).target = targetAutomaton.states.at(4).get();
    // Transitions from loc2
    targetAutomaton.states.at(2)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(2)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(2)->next['c'].at(0).target = targetAutomaton.states.at(3).get();
    targetAutomaton.states.at(2)->next['c'].at(0).guard = {learnta::ConstraintMaker(1) >= scale,
                                                           learnta::ConstraintMaker(1) <= scale};
    targetAutomaton.states.at(2)->next['c'].at(1).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(2)->next['c'].at(1).guard = {learnta::ConstraintMaker(1) > scale};
    targetAutomaton.states.at(2)->next['c'].at(2).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(2)->next['c'].at(2).guard = {learnta::ConstraintMaker(1) < scale};
    // Transitions from the accepting state
    targetAutomaton.states.at(3)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(3)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(3)->next['c'].at(0).target = targetAutomaton.states.at(4).get();
    // Transitions from the sink state
    targetAutomaton.states.at(4)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(4)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
    targetAutomaton.states.at(4)->next['c'].at(0).target = targetAutomaton.states.at(4).get();

    targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
    targetAutomaton.maxConstraints.resize(2);
    targetAutomaton.maxConstraints[0] = scale;
    targetAutomaton.maxConstraints[1] = scale;

    // simplify the target DTA
    targetAutomaton.simplifyStrong();
    targetAutomaton.simplifyWithZones();

    // Construct the complement DTA
    complementTargetAutomaton = targetAutomaton.complement(this->alphabet);
    complementTargetAutomaton.simplifyStrong();
    complementTargetAutomaton.simplifyWithZones();
  }
};
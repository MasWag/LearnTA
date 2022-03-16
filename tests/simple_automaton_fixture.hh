/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include "../include/timed_automaton.hh"

/*!
 * @brief Fixture of the DTA in the paper
 */
struct SimpleAutomatonFixture {
  SimpleAutomatonFixture() {
    automaton.states.resize(2);
    automaton.states.at(0) = std::make_shared<learnta::TAState>(true);
    automaton.states.at(0)->next['a'].resize(2);
    automaton.states.at(1) = std::make_shared<learnta::TAState>(false);
    automaton.states.at(1)->next['a'].resize(2);
    // Transitions from loc0
    automaton.states.at(0)->next['a'].at(0).target = automaton.states.at(0).get();
    automaton.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < 1};
    automaton.states.at(0)->next['a'].at(1).target = automaton.states.at(1).get();
    automaton.states.at(0)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) >= 1};
    automaton.states.at(0)->next['a'].at(1).resetVars.push_back(0);
    // Transitions from loc1
    automaton.states.at(1)->next['a'].at(0).target = automaton.states.at(0).get();
    automaton.states.at(1)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= 1};
    automaton.states.at(1)->next['a'].at(1).target = automaton.states.at(1).get();
    automaton.states.at(1)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) > 1};

    automaton.initialStates.push_back(automaton.states.at(0));
    automaton.maxConstraints.resize(1);
    automaton.maxConstraints[0] = 1;
  }

  learnta::TimedAutomaton automaton;
};

struct ComplementSimpleAutomatonFixture {
  ComplementSimpleAutomatonFixture() {
    complementAutomaton.states.resize(2);
    complementAutomaton.states.at(0) = std::make_shared<learnta::TAState>(false);
    complementAutomaton.states.at(0)->next['a'].resize(2);
    complementAutomaton.states.at(1) = std::make_shared<learnta::TAState>(true);
    complementAutomaton.states.at(1)->next['a'].resize(2);
    // Transitions from loc0
    complementAutomaton.states.at(0)->next['a'].at(0).target = complementAutomaton.states.at(0).get();
    complementAutomaton.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < 1};
    complementAutomaton.states.at(0)->next['a'].at(1).target = complementAutomaton.states.at(1).get();
    complementAutomaton.states.at(0)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) >= 1};
    complementAutomaton.states.at(0)->next['a'].at(1).resetVars.push_back(0);
    // Transitions from loc1
    complementAutomaton.states.at(1)->next['a'].at(0).target = complementAutomaton.states.at(0).get();
    complementAutomaton.states.at(1)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= 1};
    complementAutomaton.states.at(1)->next['a'].at(1).target = complementAutomaton.states.at(1).get();
    complementAutomaton.states.at(1)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) > 1};

    complementAutomaton.initialStates.push_back(complementAutomaton.states.at(0));
    complementAutomaton.maxConstraints.resize(1);
    complementAutomaton.maxConstraints[0] = 1;
  }

  learnta::TimedAutomaton complementAutomaton;
};

struct UniversalAutomatonFixture {
  UniversalAutomatonFixture() {
    universalAutomaton.states.resize(1);
    universalAutomaton.states.at(0) = std::make_shared<learnta::TAState>(true);
    universalAutomaton.states.at(0)->next['a'].resize(1);
    universalAutomaton.states.at(0)->next['a'].at(0).target = universalAutomaton.states.at(0).get();

    universalAutomaton.initialStates.push_back(universalAutomaton.states.at(0));
    universalAutomaton.maxConstraints.resize(0);
  }

  learnta::TimedAutomaton universalAutomaton;
};
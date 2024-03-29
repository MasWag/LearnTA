/**
 * @author Masaki Waga
 * @date 2022/12/13.
 */

#pragma once

#include <memory>
#include "../include/common_types.hh"
#include "../include/timed_automaton.hh"

/*!
 * @brief Fixture of a DTA with unobservable transitions
 */
struct UnobservableAutomatonFixture {
  UnobservableAutomatonFixture() {
    automaton.states.resize(2);
    automaton.states.at(0) = std::make_shared<learnta::TAState>(true);
    automaton.states.at(0)->next[learnta::UNOBSERVABLE].resize(1);
    automaton.states.at(0)->next['a'].resize(1);
    automaton.states.at(1) = std::make_shared<learnta::TAState>(false);
    automaton.states.at(1)->next['a'].resize(2);
    // Transitions from loc0
    automaton.states.at(0)->next[learnta::UNOBSERVABLE].at(0).target = automaton.states.at(1).get();
    automaton.states.at(0)->next[learnta::UNOBSERVABLE].at(0).guard = {learnta::ConstraintMaker(0) >= 1};
    automaton.states.at(0)->next[learnta::UNOBSERVABLE].at(0).resetVars.emplace_back(0, 0.0);
    automaton.states.at(0)->next['a'].at(0).target = automaton.states.at(0).get();
    automaton.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < 1};
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
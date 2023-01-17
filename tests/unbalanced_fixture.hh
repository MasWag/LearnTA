/**
 * @author Masaki Waga
 * @date 2022/12/19.
 */

#pragma once

#include "../include/timed_automaton.hh"


struct UnbalancedHypothesis20221219Fixture {
  UnbalancedHypothesis20221219Fixture() {
    // Generate the target DTA
    hypothesis.states.resize(9);
    // The initial state
    hypothesis.states.at(0) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(0)->next['a'].resize(3);
    // The states waiting for b
    hypothesis.states.at(1) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(1)->next['b'].resize(1);
    hypothesis.states.at(2) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(2)->next['b'].resize(1);
    hypothesis.states.at(3) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(3)->next['b'].resize(1);
    // The state waiting for c
    hypothesis.states.at(4) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(4)->next['c'].resize(1);
    hypothesis.states.at(5) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(5)->next['c'].resize(1);
    // The state to move to state 5 by an unobservable transition
    hypothesis.states.at(6) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(6)->next[learnta::UNOBSERVABLE].resize(1);
    hypothesis.states.at(7) = std::make_shared<learnta::TAState>(true);
    hypothesis.states.at(8) = std::make_shared<learnta::TAState>(true);

    // Transitions from loc0
    // loc0->loc1 [label="a", guard="{x0 <= 0}", reset="{x1 := 0}"]
    hypothesis.states.at(0)->next['a'].at(0).target = hypothesis.states.at(1).get();
    hypothesis.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= 0};
    hypothesis.states.at(0)->next['a'].at(0).resetVars = {{1, 0.0}};
    // loc0->loc2 [label="a", guard="{x0 > 0, x0 < 1}", reset="{x1 := 0}"]
    hypothesis.states.at(0)->next['a'].at(1).target = hypothesis.states.at(2).get();
    hypothesis.states.at(0)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) > 0,
                                                      learnta::ConstraintMaker(0) < 1};
    hypothesis.states.at(0)->next['a'].at(1).resetVars = {{1, 0.0}};
    // loc0->loc3 [label="a", guard="{x0 >= 1, x0 <= 1}", reset="{x1 := 0}"]
    hypothesis.states.at(0)->next['a'].at(2).target = hypothesis.states.at(3).get();
    hypothesis.states.at(0)->next['a'].at(2).guard = {learnta::ConstraintMaker(0) >= 1,
                                                      learnta::ConstraintMaker(0) <= 1};
    hypothesis.states.at(0)->next['a'].at(2).resetVars = {{1, 0.0}};
    // Transitions from loc1
    // loc1->loc4 [label="b", guard="{x0 >= 1, x0 <= 1, x1 >= 1, x1 <= 1}", reset="{x2 := 0}"]
    hypothesis.states.at(1)->next['b'].at(0).target = hypothesis.states.at(4).get();
    hypothesis.states.at(1)->next['b'].at(0).guard = {learnta::ConstraintMaker(0) >= 1,
                                                      learnta::ConstraintMaker(0) <= 1,
                                                      learnta::ConstraintMaker(1) >= 1,
                                                      learnta::ConstraintMaker(1) <= 1};
    hypothesis.states.at(1)->next['b'].at(0).resetVars = {{2, 0.0}};
    // loc2->loc5 [label="b", guard="{x0 >= 1, x0 <= 1, x1 > 0, x1 < 1}", reset="{x2 := 0}"]
    hypothesis.states.at(2)->next['b'].at(0).target = hypothesis.states.at(5).get();
    hypothesis.states.at(2)->next['b'].at(0).guard = {learnta::ConstraintMaker(0) >= 1,
                                                      learnta::ConstraintMaker(0) <= 1,
                                                      learnta::ConstraintMaker(1) > 0,
                                                      learnta::ConstraintMaker(1) < 1};
    hypothesis.states.at(2)->next['b'].at(0).resetVars = {{2, 0.0}};
    // loc3->loc6 [label="b", guard="{x0 >= 1, x0 <= 1, x1 <= 0}", reset="{x2 := 0}"]
    hypothesis.states.at(3)->next['b'].at(0).target = hypothesis.states.at(6).get();
    hypothesis.states.at(3)->next['b'].at(0).guard = {learnta::ConstraintMaker(0) >= 1,
                                                      learnta::ConstraintMaker(0) <= 1,
                                                      learnta::ConstraintMaker(1) <= 0};
    hypothesis.states.at(3)->next['b'].at(0).resetVars = {{2, 0.0}};
    // loc4->loc7 [label="c", guard="{x0 >= 1, x0 <= 1, x1 >= 1, x1 <= 1, x2 <= 0}"]
    hypothesis.states.at(4)->next['c'].at(0).target = hypothesis.states.at(7).get();
    hypothesis.states.at(4)->next['c'].at(0).guard = {learnta::ConstraintMaker(0) >= 1,
                                                      learnta::ConstraintMaker(0) <= 1,
                                                      learnta::ConstraintMaker(1) >= 1,
                                                      learnta::ConstraintMaker(1) <= 1,
                                                      learnta::ConstraintMaker(2) <= 0};
    // loc5->loc8 [label="c", guard="{x1 >= 1, x1 <= 1, x2 > 0}"]
    hypothesis.states.at(5)->next['c'].at(0).target = hypothesis.states.at(8).get();
    hypothesis.states.at(5)->next['c'].at(0).guard = {learnta::ConstraintMaker(1) < 1,
                                                      learnta::ConstraintMaker(2) > 0};
    // loc6->loc5 [label="ε", guard="{x0 > 1, x1 > 0, x2 > 0}", reset="{x0 := 1.5, x2 := 0.5}"]
    hypothesis.states.at(6)->next[learnta::UNOBSERVABLE].at(0).target = hypothesis.states.at(5).get();
    hypothesis.states.at(6)->next[learnta::UNOBSERVABLE].at(0).guard = {learnta::ConstraintMaker(0) > 1,
                                                                        learnta::ConstraintMaker(1) > 0,
                                                                        learnta::ConstraintMaker(2) > 0};
    hypothesis.states.at(6)->next[learnta::UNOBSERVABLE].at(0).resetVars = {{0, 1.5}, {2, 0.5}};

    hypothesis.initialStates.push_back(hypothesis.states.at(0));
    hypothesis.maxConstraints.resize(3);
    hypothesis.maxConstraints[0] = 1;
    hypothesis.maxConstraints[1] = 1;
    hypothesis.maxConstraints[2] = 1;
  }

  learnta::TimedAutomaton hypothesis;
};


struct UnbalancedHypothesis20221220Fixture {
  UnbalancedHypothesis20221220Fixture() {
    // Generate the target DTA
    hypothesis.states.resize(2);
    hypothesis.states.at(0) = std::make_shared<learnta::TAState>(false);
    hypothesis.states.at(0)->next['a'].resize(1);
    hypothesis.states.at(0)->next['b'].resize(1);
    hypothesis.states.at(0)->next['c'].resize(1);
    hypothesis.states.at(0)->next[learnta::UNOBSERVABLE].resize(1);
    hypothesis.states.at(1) = std::make_shared<learnta::TAState>(true);

    // Transitions from loc0
    hypothesis.states.at(0)->next['a'].at(0).target = hypothesis.states.at(1).get();
    hypothesis.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= 0};
    hypothesis.states.at(0)->next['a'].at(0).resetVars = {{1, 0.0}};
    hypothesis.states.at(0)->next['b'].at(0).target = hypothesis.states.at(1).get();
    hypothesis.states.at(0)->next['b'].at(0).guard = {learnta::ConstraintMaker(0) <= 0};
    hypothesis.states.at(0)->next['b'].at(0).resetVars = {{1, 0.0}};
    hypothesis.states.at(0)->next['c'].at(0).target = hypothesis.states.at(1).get();
    hypothesis.states.at(0)->next['c'].at(0).guard = {learnta::ConstraintMaker(0) <= 0};
    hypothesis.states.at(0)->next['c'].at(0).resetVars = {{1, 0.0}};
    hypothesis.states.at(0)->next[learnta::UNOBSERVABLE].at(0).target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next[learnta::UNOBSERVABLE].at(0).guard = {learnta::ConstraintMaker(0) > 0};

    hypothesis.initialStates.push_back(hypothesis.states.at(0));
    hypothesis.maxConstraints.resize(2);
    hypothesis.maxConstraints[0] = 1;
    hypothesis.maxConstraints[1] = 1;
  }

  learnta::TimedAutomaton hypothesis;
};
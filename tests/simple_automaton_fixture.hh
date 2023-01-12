/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include <sstream>

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
    automaton.states.at(0)->next['a'].at(1).resetVars.emplace_back(0, 0.0);
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

/*!
 * @brief Fixture of a DTA equivalent to the one in the paper but with an unobservable transition
 */
struct SimpleAutomatonWithOneUnobservableFixture {
  SimpleAutomatonWithOneUnobservableFixture() {
    automatonWithOneUnobservable.states.resize(3);
    automatonWithOneUnobservable.states.at(0) = std::make_shared<learnta::TAState>(true);
    automatonWithOneUnobservable.states.at(0)->next[learnta::UNOBSERVABLE].resize(1);
    automatonWithOneUnobservable.states.at(0)->next['a'].resize(1);
    automatonWithOneUnobservable.states.at(1) = std::make_shared<learnta::TAState>(true);
    automatonWithOneUnobservable.states.at(1)->next['a'].resize(1);
    automatonWithOneUnobservable.states.at(2) = std::make_shared<learnta::TAState>(false);
    automatonWithOneUnobservable.states.at(2)->next['a'].resize(2);
    // Transitions from loc0
    automatonWithOneUnobservable.states.at(0)->next[learnta::UNOBSERVABLE].at(0).target = automatonWithOneUnobservable.states.at(1).get();
    automatonWithOneUnobservable.states.at(0)->next[learnta::UNOBSERVABLE].at(0).guard = {learnta::ConstraintMaker(0) >= 1};
    automatonWithOneUnobservable.states.at(0)->next['a'].at(0).target = automatonWithOneUnobservable.states.at(0).get();
    automatonWithOneUnobservable.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < 1};
    // Transitions from loc1
    automatonWithOneUnobservable.states.at(1)->next['a'].at(0).target = automatonWithOneUnobservable.states.at(2).get();
    automatonWithOneUnobservable.states.at(1)->next['a'].at(0).resetVars.emplace_back(0, 0.0);
    // Transitions from loc2
    automatonWithOneUnobservable.states.at(2)->next['a'].at(0).target = automatonWithOneUnobservable.states.at(0).get();
    automatonWithOneUnobservable.states.at(2)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= 1};
    automatonWithOneUnobservable.states.at(2)->next['a'].at(1).target = automatonWithOneUnobservable.states.at(2).get();
    automatonWithOneUnobservable.states.at(2)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) > 1};

    automatonWithOneUnobservable.initialStates.push_back(automatonWithOneUnobservable.states.at(0));
    automatonWithOneUnobservable.maxConstraints.resize(1);
    automatonWithOneUnobservable.maxConstraints[0] = 1;
  }
  const std::vector<learnta::Alphabet> alphabet = {'a'};
  learnta::TimedAutomaton automatonWithOneUnobservable;
};

/*!
 * @brief Fixture of a DTA equivalent to the one in the paper but with two unobservable transitions
 */
struct SimpleAutomatonWithTwoUnobservableFixture {
  SimpleAutomatonWithTwoUnobservableFixture() {
    automatonWithTwoUnobservable.states.resize(4);
    automatonWithTwoUnobservable.states.at(0) = std::make_shared<learnta::TAState>(true);
    automatonWithTwoUnobservable.states.at(0)->next[learnta::UNOBSERVABLE].resize(1);
    automatonWithTwoUnobservable.states.at(0)->next['a'].resize(1);
    automatonWithTwoUnobservable.states.at(1) = std::make_shared<learnta::TAState>(true);
    automatonWithTwoUnobservable.states.at(1)->next['a'].resize(1);
    automatonWithTwoUnobservable.states.at(2) = std::make_shared<learnta::TAState>(false);
    automatonWithTwoUnobservable.states.at(2)->next[learnta::UNOBSERVABLE].resize(1);
    automatonWithTwoUnobservable.states.at(2)->next['a'].resize(1);
    automatonWithTwoUnobservable.states.at(3) = std::make_shared<learnta::TAState>(false);
    automatonWithTwoUnobservable.states.at(3)->next['a'].resize(1);
    // Transitions from loc0
    automatonWithTwoUnobservable.states.at(0)->next[learnta::UNOBSERVABLE].at(0).target = automatonWithTwoUnobservable.states.at(1).get();
    automatonWithTwoUnobservable.states.at(0)->next[learnta::UNOBSERVABLE].at(0).guard = {learnta::ConstraintMaker(0) >= 1};
    automatonWithTwoUnobservable.states.at(0)->next['a'].at(0).target = automatonWithTwoUnobservable.states.at(0).get();
    automatonWithTwoUnobservable.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < 1};
    // Transitions from loc1
    automatonWithTwoUnobservable.states.at(1)->next['a'].at(0).target = automatonWithTwoUnobservable.states.at(2).get();
    automatonWithTwoUnobservable.states.at(1)->next['a'].at(0).resetVars.emplace_back(0, 0.0);
    // Transitions from loc2
    automatonWithTwoUnobservable.states.at(2)->next['a'].at(0).target = automatonWithTwoUnobservable.states.at(0).get();
    automatonWithTwoUnobservable.states.at(2)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= 1};
    automatonWithTwoUnobservable.states.at(2)->next[learnta::UNOBSERVABLE].at(0).target = automatonWithTwoUnobservable.states.at(3).get();
    automatonWithTwoUnobservable.states.at(2)->next[learnta::UNOBSERVABLE].at(0).guard = {learnta::ConstraintMaker(0) > 1};
    // Transitions from loc3
    automatonWithTwoUnobservable.states.at(3)->next['a'].at(0).target = automatonWithTwoUnobservable.states.at(3).get();

    automatonWithTwoUnobservable.initialStates.push_back(automatonWithTwoUnobservable.states.at(0));
    automatonWithTwoUnobservable.maxConstraints.resize(1);
    automatonWithTwoUnobservable.maxConstraints[0] = 1;
  }

  learnta::TimedAutomaton automatonWithTwoUnobservable;
  const std::vector<learnta::Alphabet> alphabet = {'a'};
};

/*!
 * @brief Fixture of the DTA in the paper
 */
template<int T>
struct SimpleAutomatonFixtureT {
  SimpleAutomatonFixtureT() {
    automaton.states.resize(2);
    automaton.states.at(0) = std::make_shared<learnta::TAState>(true);
    automaton.states.at(0)->next['a'].resize(2);
    automaton.states.at(1) = std::make_shared<learnta::TAState>(false);
    automaton.states.at(1)->next['a'].resize(2);
    // Transitions from loc0
    automaton.states.at(0)->next['a'].at(0).target = automaton.states.at(0).get();
    automaton.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < T};
    automaton.states.at(0)->next['a'].at(1).target = automaton.states.at(1).get();
    automaton.states.at(0)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) >= T};
    automaton.states.at(0)->next['a'].at(1).resetVars.emplace_back(0, 0.0);
    // Transitions from loc1
    automaton.states.at(1)->next['a'].at(0).target = automaton.states.at(0).get();
    automaton.states.at(1)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= T};
    automaton.states.at(1)->next['a'].at(1).target = automaton.states.at(1).get();
    automaton.states.at(1)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) > T};

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
    complementAutomaton.states.at(0)->next['a'].at(1).resetVars.emplace_back(0, 0.0);
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

struct SimpleDTALearnedFixture {
  learnta::TimedAutomaton hypothesis;
  SimpleDTALearnedFixture() {
    using namespace learnta;
    hypothesis.states.resize(2);
    hypothesis.states.at(0) = std::make_shared<learnta::TAState>(true);
    hypothesis.states.at(1) = std::make_shared<learnta::TAState>(false);

    hypothesis.states.at(0)->next['a'].resize(6);
    hypothesis.states.at(0)->next['a'].at(0).target = hypothesis.states.at(1).get();
    hypothesis.states.at(0)->next['a'].at(0).guard = {ConstraintMaker(0) >= 2, ConstraintMaker(0) <= 2};
    hypothesis.states.at(0)->next['a'].at(0).resetVars.emplace_back(1, 0.0);

    hypothesis.states.at(0)->next['a'].at(1).target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['a'].at(1).guard = {ConstraintMaker(0) <= 0};
    hypothesis.states.at(0)->next['a'].at(1).resetVars.emplace_back(0, 0.0);

    hypothesis.states.at(0)->next['a'].at(2).target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['a'].at(2).guard = {ConstraintMaker(0) > 0, ConstraintMaker(0) < 1};

    hypothesis.states.at(0)->next['a'].at(3).target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['a'].at(3).guard = {ConstraintMaker(0) >= 1, ConstraintMaker(0) <= 1};
    hypothesis.states.at(0)->next['a'].at(3).resetVars.emplace_back(0, 1.0);

    hypothesis.states.at(0)->next['a'].at(4).target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['a'].at(4).guard = {ConstraintMaker(0) > 1, ConstraintMaker(0) < 2};

    hypothesis.states.at(0)->next['a'].at(5).target = hypothesis.states.at(1).get();
    hypothesis.states.at(0)->next['a'].at(5).guard = {ConstraintMaker(0) > 2};
    hypothesis.states.at(0)->next['a'].at(5).resetVars.emplace_back(1, 0.0);
    hypothesis.states.at(0)->next['a'].at(5).resetVars.emplace_back(0, 2.0);

    hypothesis.states.at(1)->next['a'].resize(5);
    hypothesis.states.at(1)->next['a'].at(0).target = hypothesis.states.at(0).get();
    hypothesis.states.at(1)->next['a'].at(0).guard = {ConstraintMaker(0) >= 2, ConstraintMaker(0) <= 2,
                                                      ConstraintMaker(1) <= 0};
    hypothesis.states.at(1)->next['a'].at(0).resetVars.emplace_back(0, 0.0);

    hypothesis.states.at(1)->next['a'].at(1).target = hypothesis.states.at(0).get();
    hypothesis.states.at(1)->next['a'].at(1).guard = {ConstraintMaker(0) > 2, ConstraintMaker(0) < 3,
                                                      ConstraintMaker(1) > 0, ConstraintMaker(1) < 1};
    hypothesis.states.at(1)->next['a'].at(1).resetVars.emplace_back(0, static_cast<ClockVariables>(1));

    hypothesis.states.at(1)->next['a'].at(2).target = hypothesis.states.at(0).get();
    hypothesis.states.at(1)->next['a'].at(2).guard = {ConstraintMaker(0) >= 3, ConstraintMaker(0) <= 3,
                                                      ConstraintMaker(1) >= 1, ConstraintMaker(1) <= 1};
    hypothesis.states.at(1)->next['a'].at(2).resetVars.emplace_back(0, 1.0);

    hypothesis.states.at(1)->next['a'].at(3).target = hypothesis.states.at(0).get();
    hypothesis.states.at(1)->next['a'].at(3).guard = {ConstraintMaker(0) > 3, ConstraintMaker(0) < 4,
                                                      ConstraintMaker(1) > 1, ConstraintMaker(1) < 2};
    hypothesis.states.at(1)->next['a'].at(3).resetVars.emplace_back(0, static_cast<ClockVariables>(1));

    hypothesis.states.at(1)->next['a'].at(4).target = hypothesis.states.at(0).get();
    hypothesis.states.at(1)->next['a'].at(4).guard = {ConstraintMaker(0) >= 4, ConstraintMaker(0) <= 4,
                                                      ConstraintMaker(1) >= 2, ConstraintMaker(1) <= 2};
    hypothesis.states.at(1)->next['a'].at(4).resetVars.emplace_back(0, 2.0);

    hypothesis.initialStates.push_back(hypothesis.states.at(0));
    hypothesis.maxConstraints.reserve(2);
    hypothesis.maxConstraints.push_back(4);
    hypothesis.maxConstraints.push_back(2);
  }

  void validate() const {
    std::stringstream stream;
    stream << hypothesis;
    std::string expected =
            "digraph G {\n"
            "        loc0 [init=1, match=1]\n"
            "        loc1 [init=0, match=0]\n"
            "        loc0->loc1 [label=\"a\", guard=\"{x0 >= 2, x0 <= 2}\", reset=\"{x1 := 0}\"]\n"
            "        loc0->loc0 [label=\"a\", guard=\"{x0 <= 0}\", reset=\"{x0 := 0}\"]\n"
            "        loc0->loc0 [label=\"a\", guard=\"{x0 > 0, x0 < 1}\"]\n"
            "        loc0->loc0 [label=\"a\", guard=\"{x0 >= 1, x0 <= 1}\", reset=\"{x0 := 1}\"]\n"
            "        loc0->loc0 [label=\"a\", guard=\"{x0 > 1, x0 < 2}\"]\n"
            "        loc0->loc1 [label=\"a\", guard=\"{x0 > 2}\", reset=\"{x1 := 0, x0 := 2}\"]\n"
            "        loc1->loc0 [label=\"a\", guard=\"{x0 >= 2, x0 <= 2, x1 <= 0}\", reset=\"{x0 := 0}\"]\n"
            "        loc1->loc0 [label=\"a\", guard=\"{x0 > 2, x0 < 3, x1 > 0, x1 < 1}\", reset=\"{x0 := x1}\"]\n"
            "        loc1->loc0 [label=\"a\", guard=\"{x0 >= 3, x0 <= 3, x1 >= 1, x1 <= 1}\", reset=\"{x0 := 1}\"]\n"
            "        loc1->loc0 [label=\"a\", guard=\"{x0 > 3, x0 < 4, x1 > 1, x1 < 2}\", reset=\"{x0 := x1}\"]\n"
            "        loc1->loc0 [label=\"a\", guard=\"{x0 >= 4, x0 <= 4, x1 >= 2, x1 <= 2}\", reset=\"{x0 := 2}\"]\n"
            "}\n";
    BOOST_CHECK_EQUAL(expected, stream.str());
  }
};
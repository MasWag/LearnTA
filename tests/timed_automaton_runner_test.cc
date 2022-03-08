/**
 * @author Masaki Waga
 * @date 2022/03/07.
 */

#include <boost/test/unit_test.hpp>

#include "../include/timed_automaton.hh"
#include "../include/timed_automaton_runner.hh"

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

BOOST_AUTO_TEST_SUITE(TimedAutomatonRunnerTest)

  using namespace learnta;

  BOOST_FIXTURE_TEST_CASE(step, SimpleAutomatonFixture) {
    TimedAutomatonRunner runner{this->automaton};
    runner.pre();
    BOOST_CHECK(runner.step(0.3));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(runner.step(0.8));
    BOOST_CHECK(!runner.step('a'));
    BOOST_CHECK(!runner.step(0.3));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(runner.step(0.5));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(runner.step(0.3));
    BOOST_CHECK(!runner.step('a'));
    BOOST_CHECK(!runner.step(1.2));
    BOOST_CHECK(!runner.step('a'));
    BOOST_CHECK(!runner.step(0.3));
    BOOST_CHECK(!runner.step('a'));
    runner.post();
  }

BOOST_AUTO_TEST_SUITE_END()

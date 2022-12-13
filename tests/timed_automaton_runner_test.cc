/**
 * @author Masaki Waga
 * @date 2022/03/07.
 */

#include <boost/test/unit_test.hpp>

#include "../include/timed_automaton_runner.hh"

#include "simple_automaton_fixture.hh"
#include "unobservable_automaton_fixture.hh"

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
  BOOST_FIXTURE_TEST_CASE(print, SimpleAutomatonFixture) {
    std::stringstream stream;
    stream << this->automaton;
    std::string expected = "digraph G {\n";
    expected += "        loc0 [init=1, match=1]\n";
    expected += "        loc1 [init=0, match=0]\n";
    expected += "        loc0->loc0 [label=\"a\", guard=\"{x0 < 1}\"]\n";
    expected += "        loc0->loc1 [label=\"a\", guard=\"{x0 >= 1}\", reset=\"{x0 := 0}\"]\n";
    expected += "        loc1->loc0 [label=\"a\", guard=\"{x0 <= 1}\"]\n";
    expected += "        loc1->loc1 [label=\"a\", guard=\"{x0 > 1}\"]\n";
    expected += "}\n";
    BOOST_CHECK_EQUAL(expected, stream.str());
  }

  BOOST_FIXTURE_TEST_CASE(stepUnobservable, UnobservableAutomatonFixture) {
    TimedAutomatonRunner runner{this->automaton};
    runner.pre();
    BOOST_CHECK(runner.step(0.3));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(!runner.step(0.8));
    BOOST_CHECK(!runner.step(0.3));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(runner.step(0.5));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(!runner.step(0.3));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(!runner.step(1.2));
    BOOST_CHECK(runner.step('a'));
    BOOST_CHECK(runner.step(0.3));
    BOOST_CHECK(!runner.step(0.5));
    BOOST_CHECK(!runner.step(0.9));
    BOOST_CHECK(!runner.step('a'));
    runner.post();
  }
BOOST_AUTO_TEST_SUITE_END()

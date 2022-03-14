/**
 * @author Masaki Waga
 * @date 2022/03/07.
 */

#include <boost/test/unit_test.hpp>

#include "../include/timed_automaton.hh"
#include "../include/timed_automaton_runner.hh"

#include "simple_automaton_fixture.hh"

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

/**
 * @author Masaki Waga
 * @date 2022/03/16.
 */

#include <boost/test/unit_test.hpp>

#include "../include/timed_automata_equivalence_oracle.hh"
#include "../include/timed_automaton_runner.hh"
#include "simple_automaton_fixture.hh"

BOOST_AUTO_TEST_SUITE(TimedAutomataEquivalenceOracleTest)

  using namespace learnta;
  struct Fixture : public SimpleAutomatonFixture, public UniversalAutomatonFixture, public ComplementSimpleAutomatonFixture {
  };

  BOOST_FIXTURE_TEST_CASE(query, Fixture) {
    auto oracle = ComplementTimedAutomataEquivalenceOracle{this->automaton, this->complementAutomaton, {'a'}};

    BOOST_CHECK(oracle.findCounterExample(this->universalAutomaton));
    auto counterexample = oracle.findCounterExample(this->universalAutomaton).value();
    BOOST_CHECK_EQUAL(1, counterexample.wordSize());
    auto complementRunner = TimedAutomatonRunner{this->complementAutomaton};
    auto universalRunner = TimedAutomatonRunner{this->universalAutomaton};
    BOOST_CHECK(!complementRunner.step(counterexample.getDurations().at(0)));
    BOOST_CHECK(universalRunner.step(counterexample.getDurations().at(0)));
    BOOST_CHECK(complementRunner.step(counterexample.getWord().at(0)));
    BOOST_CHECK(universalRunner.step(counterexample.getWord().at(0)));
    BOOST_CHECK(complementRunner.step(counterexample.getDurations().at(1)));
    BOOST_CHECK(universalRunner.step(counterexample.getDurations().at(1)));

    BOOST_CHECK(!oracle.findCounterExample(this->automaton));
  }

BOOST_AUTO_TEST_SUITE_END()
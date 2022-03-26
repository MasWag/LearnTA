/**
 * @author Masaki Waga
 * @date 2022/03/22.
 */

#include <iostream>
#include <boost/test/unit_test.hpp>

#define private public
#define protected public

#include "../include/timed_automaton_runner.hh"
#include "../include/symbolic_membership_oracle.hh"
#include "../include/equivalence_oracle.hh"
#include "../include/timed_automata_equivalence_oracle.hh"

#include "simple_automaton_fixture.hh"
#include "simple_observation_table_keys_fixture.hh"
#include "observation_table.hh"

using namespace learnta;

BOOST_AUTO_TEST_SUITE(ObservationTableTest)

  struct SimpleAutomatonOracleFixture : public SimpleAutomatonFixture, ComplementSimpleAutomatonFixture, SimpleObservationTableKeysFixture {
    std::unique_ptr<learnta::SymbolicMembershipOracle> memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(
            std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner{this->automaton}));
    const std::vector<Alphabet> alphabet = {'a'};
    ObservationTable observationTable;

    SimpleAutomatonOracleFixture() :
            observationTable(alphabet,
                             std::make_unique<learnta::SymbolicMembershipOracle>(
                                     std::unique_ptr<learnta::SUL>(
                                             new learnta::TimedAutomatonRunner{this->automaton}))) {}
  };


  BOOST_FIXTURE_TEST_CASE(initialHypothesis, SimpleAutomatonOracleFixture) {
    BOOST_CHECK(observationTable.close());
    BOOST_CHECK(observationTable.consistent());
    BOOST_CHECK(observationTable.exteriorConsistent());

    BOOST_CHECK_EQUAL(3, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.suffixes.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.discreteSuccessors.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.discreteSuccessors.at(std::make_pair(0, 'a')));
    BOOST_CHECK_EQUAL(1, this->observationTable.continuousSuccessors.size());
    BOOST_CHECK_EQUAL(2, this->observationTable.continuousSuccessors.at(0));

    // All the cells should be "top"
    for (const auto &prefix: this->observationTable.prefixes) {
      const auto concat = prefix + this->observationTable.suffixes.at(0);
      auto result = this->memOracle->query(concat);
      BOOST_CHECK_EQUAL(1, result.size());
      BOOST_CHECK_EQUAL(concat.getTimedCondition(), result.front());
    }

    const auto hypothesis = this->observationTable.generateHypothesis();

    BOOST_CHECK_EQUAL(1, hypothesis.stateSize());
    BOOST_CHECK_EQUAL(1, hypothesis.initialStates.size());
    BOOST_CHECK(hypothesis.states.front()->isMatch);
    BOOST_CHECK_EQUAL(1, hypothesis.states.front()->next.at('a').size());
    BOOST_CHECK(hypothesis.states.front()->next.at('a').front().resetVars.empty());
    BOOST_CHECK(hypothesis.states.front()->next.at('a').front().guard.empty());
    BOOST_CHECK(hypothesis.maxConstraints.empty());
  }

  BOOST_FIXTURE_TEST_CASE(secondHypothesis, SimpleAutomatonOracleFixture) {
    this->observationTable.addCounterExample(ForwardRegionalElementaryLanguage{{"a", TimedCondition{{1, 0}}},
                                                                               FractionalOrder{{0, 0}}});
    BOOST_CHECK_EQUAL(9, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(4, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.suffixes.size());
    // The observation table is already closed because P already contains 1 a
    BOOST_CHECK(this->observationTable.close());
    BOOST_CHECK(!this->observationTable.consistent());
    BOOST_CHECK_EQUAL(9, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(4, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(2, this->observationTable.suffixes.size());
    BOOST_CHECK(this->observationTable.close());
    BOOST_CHECK(!this->observationTable.consistent());
    BOOST_CHECK_EQUAL(9, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(4, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(3, this->observationTable.suffixes.size());
    BOOST_CHECK_EQUAL(s1, this->observationTable.suffixes.at(0));
    BOOST_CHECK_EQUAL(s2, this->observationTable.suffixes.at(1));
    BOOST_CHECK_EQUAL(s3, this->observationTable.suffixes.at(2));
    // We have an unexpected behavior here.
     BOOST_CHECK(!this->observationTable.close());
    BOOST_CHECK_EQUAL(11, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(5, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(3, this->observationTable.suffixes.size());
    BOOST_CHECK(!this->observationTable.close());
    BOOST_CHECK_EQUAL(13, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(6, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(3, this->observationTable.suffixes.size());
    BOOST_CHECK(!this->observationTable.close());
    BOOST_CHECK_EQUAL(15, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(7, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(3, this->observationTable.suffixes.size());
    BOOST_CHECK(this->observationTable.close());
    BOOST_CHECK(this->observationTable.consistent());
    BOOST_CHECK(!this->observationTable.exteriorConsistent());
    BOOST_CHECK_EQUAL(17, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(8, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(3, this->observationTable.suffixes.size());
    BOOST_CHECK(this->observationTable.close());
    BOOST_CHECK(this->observationTable.consistent());
    BOOST_CHECK(this->observationTable.exteriorConsistent());

    const auto hypothesis = this->observationTable.generateHypothesis();
    this->observationTable.printDetail(std::cout);
    std::cout << hypothesis << std::endl;

    BOOST_CHECK_EQUAL(2, hypothesis.stateSize());
    BOOST_CHECK_EQUAL(1, hypothesis.initialStates.size());
    BOOST_CHECK(hypothesis.states.front()->isMatch);

    BOOST_CHECK_EQUAL(2, hypothesis.maxConstraints.size());
  }

BOOST_AUTO_TEST_SUITE_END()

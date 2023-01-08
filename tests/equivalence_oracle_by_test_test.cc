/**
 * @author Masaki Waga
 * @date 2023/01/08.
 */

#include <boost/test/unit_test.hpp>

#include "../include/equivalence_oracle_by_test.hh"
#include "simple_automaton_fixture.hh"

BOOST_AUTO_TEST_SUITE(EquivalenceOracleByTestTest)
  using namespace learnta;
  struct Fixture
          : public SimpleAutomatonFixture, public UniversalAutomatonFixture, public ComplementSimpleAutomatonFixture {
  };

  BOOST_FIXTURE_TEST_CASE(query, Fixture) {
    auto oracle = EquivalenceOracleByTest{this->automaton};
    // 1 a 0 is an exact counter example
    // We feed a redundant test input
    oracle.push_back(TimedWord{"aa", {1, 0.5, 0.5}});

    BOOST_CHECK(oracle.findCounterExample(this->universalAutomaton));
    auto counterexample = oracle.findCounterExample(this->universalAutomaton).value();
    BOOST_CHECK_EQUAL("a", counterexample.getWord());
    std::vector<double> expectedDurations = {1.0, 0};
    BOOST_TEST(expectedDurations == counterexample.getDurations(), boost::test_tools::per_element());
  }
BOOST_AUTO_TEST_SUITE_END()
/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#include <boost/test/unit_test.hpp>

#include "../include/equivalence.hh"
#include "../include/symbolic_membership_oracle.hh"
#include "../include/timed_automaton_runner.hh"

#include "simple_automaton_fixture.hh"
#include "simple_observation_table_keys_fixture.hh"


BOOST_AUTO_TEST_SUITE(EquivalenceTest)

  using namespace learnta;

  struct Fixture : public SimpleObservationTableKeysFixture, SimpleAutomatonFixture {
    std::unique_ptr<learnta::SymbolicMembershipOracle> oracle;

    Fixture() {
      auto runner = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner{this->automaton});
      this->oracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(runner));
    }
  };

  BOOST_FIXTURE_TEST_CASE(p1p2s1s2EquivalenceByTop, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2};
    std::vector<TimedConditionSet> p1Row, p2Row;
    for (const auto& suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p2Row.push_back(this->oracle->query(p2 + suffix));
    }
    std::vector<std::pair<std::size_t, std::size_t>> renaming = {};
    BOOST_CHECK(equivalence(p1, p1Row, p2, p2Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p1p2s1s2s3EquivalenceByTop, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2, s3};
    std::vector<TimedConditionSet> p1Row, p2Row;
    for (const auto& suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p2Row.push_back(this->oracle->query(p2 + suffix));
    }
    std::vector<std::pair<std::size_t, std::size_t>> renaming = {};

    BOOST_CHECK(!equivalence(p1, p1Row, p2, p2Row, suffixes, renaming));
  }

BOOST_AUTO_TEST_SUITE_END()

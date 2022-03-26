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
    for (const auto &suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p2Row.push_back(this->oracle->query(p2 + suffix));
    }
    RenamingRelation renaming = {};
    BOOST_CHECK(equivalence(p1, p1Row, p2, p2Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p1p2s1s2s3EquivalenceByTop, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2, s3};
    std::vector<TimedConditionSet> p1Row, p2Row;
    for (const auto &suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p2Row.push_back(this->oracle->query(p2 + suffix));
    }
    RenamingRelation renaming = {};

    BOOST_CHECK(!equivalence(p1, p1Row, p2, p2Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p1p9s1EquivalenceByTop, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1};
    std::vector<TimedConditionSet> p1Row, p9Row;
    for (const auto &suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p9Row.push_back(this->oracle->query(p9 + suffix));
    }
    RenamingRelation renaming = {};

    BOOST_CHECK(equivalence(p1, p1Row, p9, p9Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p1p10s1EquivalenceByTop, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1};
    std::vector<TimedConditionSet> p1Row, p10Row;
    for (const auto &suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p10Row.push_back(this->oracle->query(p10 + suffix));
    }
    RenamingRelation renaming = {};

    BOOST_CHECK(equivalence(p1, p1Row, p10, p10Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p1p10s1s2EquivalenceByTop, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2};
    std::vector<TimedConditionSet> p1Row, p10Row;
    for (const auto &suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p10Row.push_back(this->oracle->query(p10 + suffix));
    }
    RenamingRelation renaming = {};

    BOOST_CHECK(equivalence(p1, p1Row, p10, p10Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p1p10s1s2FindEquivalence, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2};
    std::vector<TimedConditionSet> p1Row, p10Row;
    for (const auto &suffix: suffixes) {
      p1Row.push_back(this->oracle->query(p1 + suffix));
      p10Row.push_back(this->oracle->query(p10 + suffix));
    }
    const auto renamingOpt = findEquivalentRenaming(p1, p1Row, p10, p10Row, suffixes);

    BOOST_REQUIRE(renamingOpt);
    BOOST_CHECK(renamingOpt->empty());
  }

  BOOST_FIXTURE_TEST_CASE(p2p10s1s2s3FindEquivalence, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2, s3};
    std::vector<TimedConditionSet> p2Row, p10Row;
    for (const auto &suffix: suffixes) {
      p2Row.push_back(this->oracle->query(p2 + suffix));
      p10Row.push_back(this->oracle->query(p10 + suffix));
    }

    const auto renamingOpt = findEquivalentRenaming(p2, p2Row, p10, p10Row, suffixes);

    BOOST_REQUIRE(renamingOpt);
    BOOST_CHECK_EQUAL(1, renamingOpt->size());
    // BOOST_CHECK_EQUAL(std::make_pair(1, 0), renamingOpt->front());
  }

  BOOST_FIXTURE_TEST_CASE(p2p13s1s2s3Equivalence, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2, s3};
    std::vector<TimedConditionSet> p2Row, p13Row;
    for (const auto &suffix: suffixes) {
      p2Row.push_back(this->oracle->query(p2 + suffix));
      p13Row.push_back(this->oracle->query(p13 + suffix));
    }
    // tau_0 + tau'_0 in p2 is equal to \tau_1 + \tau_2 + \tau'_0 in p13
    RenamingRelation renaming = {{std::make_pair(0, 1)}};

    BOOST_CHECK(equivalence(p2, p2Row, p13, p13Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p2p13s1s2s3FindEquivalence, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2, s3};
    std::vector<TimedConditionSet> p2Row, p13Row;
    for (const auto &suffix: suffixes) {
      p2Row.push_back(this->oracle->query(p2 + suffix));
      p13Row.push_back(this->oracle->query(p13 + suffix));
    }

    const auto renamingOpt = findEquivalentRenaming(p2, p2Row, p13, p13Row, suffixes);

    BOOST_REQUIRE(renamingOpt);
    // tau_0 + tau'_0 in p2 is equal to \tau_1 + \tau_2 + \tau'_0 in p13
    BOOST_REQUIRE_EQUAL(1, renamingOpt->size());
    BOOST_CHECK_EQUAL(0, renamingOpt->front().first);
    BOOST_CHECK_EQUAL(1, renamingOpt->front().second);
  }

  BOOST_FIXTURE_TEST_CASE(p13p2s1s2s3Equivalence, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2, s3};
    std::vector<TimedConditionSet> p2Row, p13Row;
    for (const auto &suffix: suffixes) {
      p2Row.push_back(this->oracle->query(p2 + suffix));
      p13Row.push_back(this->oracle->query(p13 + suffix));
    }
    // tau_0 + tau'_0 in p2 is equal to \tau_1 + \tau_2 + \tau'_0 in p13
    RenamingRelation renaming = {{std::make_pair(1, 0)}};

    BOOST_CHECK(equivalence(p13, p13Row, p2, p2Row, suffixes, renaming));
  }

  BOOST_FIXTURE_TEST_CASE(p13p2s1s2s3FindEquivalence, Fixture) {
    std::vector<BackwardRegionalElementaryLanguage> suffixes = {s1, s2, s3};
    std::vector<TimedConditionSet> p2Row, p13Row;
    for (const auto &suffix: suffixes) {
      p2Row.push_back(this->oracle->query(p2 + suffix));
      p13Row.push_back(this->oracle->query(p13 + suffix));
    }

    const auto renamingOpt = findEquivalentRenaming(p13, p13Row, p2, p2Row, suffixes);

    BOOST_REQUIRE(renamingOpt);
    // tau_0 + tau'_0 in p2 is equal to \tau_1 + \tau_2 + \tau'_0 in p13
    BOOST_REQUIRE_EQUAL(1, renamingOpt->size());
    BOOST_CHECK_EQUAL(1, renamingOpt->front().first);
    BOOST_CHECK_EQUAL(0, renamingOpt->front().second);
  }

BOOST_AUTO_TEST_SUITE_END()

/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#include <boost/test/unit_test.hpp>

#define protected public

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

  void assertGraph(const TimedCondition& left, const TimedCondition& right, const RenamingGraph& graph) {
    const auto &v1Edges = graph.first;
    const auto &v2Edges = graph.second;
    const auto N = left.size();
    const auto M = right.size();

    for (std::size_t i = 0; i < left.size(); ++i) {
      BOOST_TEST(is_strict_ascending(v1Edges.at(i)));
      for (std::size_t j = 0; j < right.size(); ++j) {
        BOOST_TEST(is_strict_ascending(v2Edges.at(j)));
        if (left.getUpperBound(i, N - 1) == right.getUpperBound(j, M - 1)) {
          BOOST_TEST(std::binary_search(v1Edges.at(i).begin(), v1Edges.at(i).end(), j));
          BOOST_TEST(std::binary_search(v2Edges.at(j).begin(), v2Edges.at(j).end(), i));
        } else {
          BOOST_TEST(!std::binary_search(v1Edges.at(i).begin(), v1Edges.at(i).end(), j));
          BOOST_TEST(!std::binary_search(v2Edges.at(j).begin(), v2Edges.at(j).end(), i));
        }
      }
    }
  }

  BOOST_FIXTURE_TEST_CASE(toGraphFixtureTest, Fixture) {
    const std::vector<ForwardRegionalElementaryLanguage> prefixes = {p1, p2, p3, p4, p5, p6, p7, p9, p10, p13};
    for (const auto& left: prefixes) {
      for (const auto& right: prefixes) {
        assertGraph(left.getTimedCondition(), right.getTimedCondition(),
                    toGraph(left.getTimedCondition(), right.getTimedCondition()));
      }
    }
  }

  BOOST_AUTO_TEST_CASE(equivalenceBug20220928) {
    std::stringstream stream;
    //         [0] = (first = 2, second = 2)
    RenamingRelation renamingRelation;
    renamingRelation.emplace_back(2, 2);

    TimedCondition leftCondition {Zone::top(4)};
    leftCondition.restrictLowerBound(0, 0, Bounds{-2, true});
    leftCondition.restrictUpperBound(0, 0, Bounds{2, true});
    leftCondition.restrictLowerBound(0, 1, Bounds{-2, false});
    leftCondition.restrictUpperBound(0, 1, Bounds{3, false});
    leftCondition.restrictLowerBound(0, 2, Bounds{-3, false});
    leftCondition.restrictUpperBound(0, 2, Bounds{4, false});
    leftCondition.restrictLowerBound(1, 1, Bounds{0, false});
    leftCondition.restrictUpperBound(1, 1, Bounds{1, false});
    leftCondition.restrictLowerBound(1, 2, Bounds{-1, false});
    leftCondition.restrictUpperBound(1, 2, Bounds{2, false});
    leftCondition.restrictLowerBound(2, 2, Bounds{0, false});
    leftCondition.restrictUpperBound(2, 2, Bounds{1, false});
    ElementaryLanguage left {"bb", leftCondition};
    std::string leftAsString = "(bb, 2 <= T_{0, 0}  <= 2 && 2 < T_{0, 1}  < 3 && 3 < T_{0, 2}  < 4 && -0 < T_{1, 1}  < 1 && 1 < T_{1, 2}  < 2 && -0 < T_{2, 2}  < 1)";
    stream << left;
    BOOST_CHECK_EQUAL(leftAsString, stream.str());
    stream.str("");

    TimedCondition rightCondition {Zone::top(4)};
    rightCondition.restrictLowerBound(0, 0, Bounds{-2, false});
    rightCondition.restrictUpperBound(0, 0, Bounds{3, false});
    rightCondition.restrictLowerBound(0, 1, Bounds{-3, true});
    rightCondition.restrictUpperBound(0, 1, Bounds{3, true});
    rightCondition.restrictLowerBound(0, 2, Bounds{-3, false});
    rightCondition.restrictUpperBound(0, 2, Bounds{4, false});
    rightCondition.restrictLowerBound(1, 1, Bounds{0, false});
    rightCondition.restrictUpperBound(1, 1, Bounds{1, false});
    rightCondition.restrictLowerBound(1, 2, Bounds{-1, true});
    rightCondition.restrictUpperBound(1, 2, Bounds{1, true});
    rightCondition.restrictLowerBound(2, 2, Bounds{0, false});
    rightCondition.restrictUpperBound(2, 2, Bounds{1, false});
    ElementaryLanguage right {"bb", rightCondition};
    std::string rightAsString = "(bb, 2 < T_{0, 0}  < 3 && 3 <= T_{0, 1}  <= 3 && 3 < T_{0, 2}  < 4 && -0 < T_{1, 1}  < 1 && 1 <= T_{1, 2}  <= 1 && -0 < T_{2, 2}  < 1)";
    stream << right;
    BOOST_CHECK_EQUAL(rightAsString, stream.str());
    stream.str("");

    std::vector<BackwardRegionalElementaryLanguage> suffixes;
    std::vector<TimedConditionSet> leftRow, rightRow;
    suffixes.reserve(8);
    leftRow.reserve(8);
    rightRow.reserve(8);

    suffixes.emplace_back();
    stream << suffixes.at(0);
    BOOST_CHECK_EQUAL("(, -0 <= T_{0, 0}  <= 0, 0 <= {x0, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    rightRow.emplace_back();
    BOOST_TEST(leftRow.at(0).empty());
    BOOST_TEST(rightRow.at(0).empty());

    suffixes.push_back(suffixes.at(0).predecessor('a').predecessor().predecessor());
    stream << suffixes.at(1);
    BOOST_CHECK_EQUAL("(a, 1 <= T_{0, 0}  <= 1 && 1 <= T_{0, 1}  <= 1 && -0 <= T_{1, 1}  <= 0, 0 <= {x0, x1, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    leftRow.at(1).push_back(TimedCondition{Zone::top(5)});
    leftRow.at(1).front().restrictLowerBound(0, 0, Bounds{-2, true});
    leftRow.at(1).front().restrictUpperBound(0, 0, Bounds{2, true});
    leftRow.at(1).front().restrictLowerBound(0, 1, Bounds{-2, false});
    leftRow.at(1).front().restrictUpperBound(0, 1, Bounds{3, false});
    leftRow.at(1).front().restrictLowerBound(0, 2, Bounds{-4, false});
    leftRow.at(1).front().restrictUpperBound(0, 2, Bounds{5, false});
    leftRow.at(1).front().restrictLowerBound(0, 3, Bounds{-4, false});
    leftRow.at(1).front().restrictUpperBound(0, 3, Bounds{5, false});
    leftRow.at(1).front().restrictLowerBound(1, 1, Bounds{0, false});
    leftRow.at(1).front().restrictUpperBound(1, 1, Bounds{1, false});
    leftRow.at(1).front().restrictLowerBound(1, 2, Bounds{-2, false});
    leftRow.at(1).front().restrictUpperBound(1, 2, Bounds{3, false});
    leftRow.at(1).front().restrictLowerBound(1, 3, Bounds{-2, false});
    leftRow.at(1).front().restrictUpperBound(1, 3, Bounds{3, false});
    leftRow.at(1).front().restrictLowerBound(2, 2, Bounds{-1, false});
    leftRow.at(1).front().restrictUpperBound(2, 2, Bounds{2, false});
    leftRow.at(1).front().restrictLowerBound(2, 3, Bounds{-1, false});
    leftRow.at(1).front().restrictUpperBound(2, 3, Bounds{2, false});
    leftRow.at(1).front().restrictLowerBound(3, 3, Bounds{0, true});
    leftRow.at(1).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, leftRow.at(1).size());
    stream << leftRow.at(1).front();
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 < T_{0, 1}  < 3 && 4 < T_{0, 2}  < 5 && 4 < T_{0, 3}  < 5 && -0 < T_{1, 1}  < 1 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && 1 < T_{2, 2}  < 2 && 1 < T_{2, 3}  < 2 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");
    rightRow.emplace_back();
    rightRow.at(1).push_back(TimedCondition{Zone::top(5)});
    rightRow.at(1).front().restrictLowerBound(0, 0, Bounds{-2, false});
    rightRow.at(1).front().restrictUpperBound(0, 0, Bounds{3, false});
    rightRow.at(1).front().restrictLowerBound(0, 1, Bounds{-3, true});
    rightRow.at(1).front().restrictUpperBound(0, 1, Bounds{3, true});
    rightRow.at(1).front().restrictLowerBound(0, 2, Bounds{-4, false});
    rightRow.at(1).front().restrictUpperBound(0, 2, Bounds{5, false});
    rightRow.at(1).front().restrictLowerBound(0, 3, Bounds{-4, false});
    rightRow.at(1).front().restrictUpperBound(0, 3, Bounds{5, false});
    rightRow.at(1).front().restrictLowerBound(1, 1, Bounds{0, false});
    rightRow.at(1).front().restrictUpperBound(1, 1, Bounds{1, false});
    rightRow.at(1).front().restrictLowerBound(1, 2, Bounds{-2, true});
    rightRow.at(1).front().restrictUpperBound(1, 2, Bounds{2, true});
    rightRow.at(1).front().restrictLowerBound(1, 3, Bounds{-2, true});
    rightRow.at(1).front().restrictUpperBound(1, 3, Bounds{2, true});
    rightRow.at(1).front().restrictLowerBound(2, 2, Bounds{-1, false});
    rightRow.at(1).front().restrictUpperBound(2, 2, Bounds{2, false});
    rightRow.at(1).front().restrictLowerBound(2, 3, Bounds{-1, false});
    rightRow.at(1).front().restrictUpperBound(2, 3, Bounds{2, false});
    rightRow.at(1).front().restrictLowerBound(3, 3, Bounds{0, true});
    rightRow.at(1).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, rightRow.at(1).size());
    stream << rightRow.at(1).front();
    BOOST_CHECK_EQUAL("2 < T_{0, 0}  < 3 && 3 <= T_{0, 1}  <= 3 && 4 < T_{0, 2}  < 5 && 4 < T_{0, 3}  < 5 && -0 < T_{1, 1}  < 1 && 2 <= T_{1, 2}  <= 2 && 2 <= T_{1, 3}  <= 2 && 1 < T_{2, 2}  < 2 && 1 < T_{2, 3}  < 2 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");

    suffixes.emplace_back(suffixes.at(0).predecessor('a'));
    stream << suffixes.at(2);
    BOOST_CHECK_EQUAL("(a, -0 <= T_{0, 0}  <= 0 && -0 <= T_{0, 1}  <= 0 && -0 <= T_{1, 1}  <= 0, 0 <= {x0, x1, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    rightRow.emplace_back();
    BOOST_TEST(leftRow.at(2).empty());
    BOOST_TEST(rightRow.at(2).empty());

    suffixes.push_back(suffixes.at(2).predecessor());
    stream << suffixes.at(3);
    BOOST_CHECK_EQUAL("(a, -0 < T_{0, 0}  < 1 && -0 < T_{0, 1}  < 1 && -0 <= T_{1, 1}  <= 0, 0 < {x0, x1, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    leftRow.at(3).push_back(TimedCondition{Zone::top(5)});
    leftRow.at(3).front().restrictLowerBound(0, 0, Bounds{-2, true});
    leftRow.at(3).front().restrictUpperBound(0, 0, Bounds{2, true});
    leftRow.at(3).front().restrictLowerBound(0, 1, Bounds{-2, false});
    leftRow.at(3).front().restrictUpperBound(0, 1, Bounds{3, false});
    leftRow.at(3).front().restrictLowerBound(0, 2, Bounds{-3, false});
    leftRow.at(3).front().restrictUpperBound(0, 2, Bounds{5, false});
    leftRow.at(3).front().restrictLowerBound(0, 3, Bounds{-3, false});
    leftRow.at(3).front().restrictUpperBound(0, 3, Bounds{5, false});
    leftRow.at(3).front().restrictLowerBound(1, 1, Bounds{0, false});
    leftRow.at(3).front().restrictUpperBound(1, 1, Bounds{1, false});
    leftRow.at(3).front().restrictLowerBound(1, 2, Bounds{-1, false});
    leftRow.at(3).front().restrictUpperBound(1, 2, Bounds{3, false});
    leftRow.at(3).front().restrictLowerBound(1, 3, Bounds{-1, false});
    leftRow.at(3).front().restrictUpperBound(1, 3, Bounds{3, false});
    leftRow.at(3).front().restrictLowerBound(2, 2, Bounds{-1, false});
    leftRow.at(3).front().restrictUpperBound(2, 2, Bounds{2, false});
    leftRow.at(3).front().restrictLowerBound(2, 3, Bounds{-1, false});
    leftRow.at(3).front().restrictUpperBound(2, 3, Bounds{2, false});
    leftRow.at(3).front().restrictLowerBound(3, 3, Bounds{0, true});
    leftRow.at(3).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, leftRow.at(3).size());
    stream << leftRow.at(3).front();
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 < T_{0, 1}  < 3 && 3 < T_{0, 2}  < 5 && 3 < T_{0, 3}  < 5 && -0 < T_{1, 1}  < 1 && 1 < T_{1, 2}  < 3 && 1 < T_{1, 3}  < 3 && 1 < T_{2, 2}  < 2 && 1 < T_{2, 3}  < 2 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");
    rightRow.emplace_back();
    rightRow.at(3).push_back(TimedCondition{Zone::top(5)});
    rightRow.at(3).front().restrictLowerBound(0, 0, Bounds{-2, false});
    rightRow.at(3).front().restrictUpperBound(0, 0, Bounds{3, false});
    rightRow.at(3).front().restrictLowerBound(0, 1, Bounds{-3, true});
    rightRow.at(3).front().restrictUpperBound(0, 1, Bounds{3, true});
    rightRow.at(3).front().restrictLowerBound(0, 2, Bounds{-4, false});
    rightRow.at(3).front().restrictUpperBound(0, 2, Bounds{5, false});
    rightRow.at(3).front().restrictLowerBound(0, 3, Bounds{-4, false});
    rightRow.at(3).front().restrictUpperBound(0, 3, Bounds{5, false});
    rightRow.at(3).front().restrictLowerBound(1, 1, Bounds{0, false});
    rightRow.at(3).front().restrictUpperBound(1, 1, Bounds{1, false});
    rightRow.at(3).front().restrictLowerBound(1, 2, Bounds{-1, false});
    rightRow.at(3).front().restrictUpperBound(1, 2, Bounds{2, false});
    rightRow.at(3).front().restrictLowerBound(1, 3, Bounds{-1, false});
    rightRow.at(3).front().restrictUpperBound(1, 3, Bounds{2, false});
    rightRow.at(3).front().restrictLowerBound(2, 2, Bounds{-1, false});
    rightRow.at(3).front().restrictUpperBound(2, 2, Bounds{2, false});
    rightRow.at(3).front().restrictLowerBound(2, 3, Bounds{-1, false});
    rightRow.at(3).front().restrictUpperBound(2, 3, Bounds{2, false});
    rightRow.at(3).front().restrictLowerBound(3, 3, Bounds{0, true});
    rightRow.at(3).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, rightRow.at(3).size());
    stream << rightRow.at(3).front();
    BOOST_CHECK_EQUAL("2 < T_{0, 0}  < 3 && 3 <= T_{0, 1}  <= 3 && 4 < T_{0, 2}  < 5 && 4 < T_{0, 3}  < 5 && -0 < T_{1, 1}  < 1 && 1 < T_{1, 2}  < 2 && 1 < T_{1, 3}  < 2 && 1 < T_{2, 2}  < 2 && 1 < T_{2, 3}  < 2 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");

    suffixes.push_back(suffixes.at(3).predecessor().predecessor());
    stream << suffixes.at(4);
    BOOST_CHECK_EQUAL("(a, 1 < T_{0, 0}  < 2 && 1 < T_{0, 1}  < 2 && -0 <= T_{1, 1}  <= 0, 0 < {x0, x1, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    leftRow.at(4).push_back(TimedCondition{Zone::top(5)});
    leftRow.at(4).front().restrictLowerBound(0, 0, Bounds{-2, true});
    leftRow.at(4).front().restrictUpperBound(0, 0, Bounds{2, true});
    leftRow.at(4).front().restrictLowerBound(0, 1, Bounds{-2, false});
    leftRow.at(4).front().restrictUpperBound(0, 1, Bounds{3, false});
    leftRow.at(4).front().restrictLowerBound(0, 2, Bounds{-4, false});
    leftRow.at(4).front().restrictUpperBound(0, 2, Bounds{6, false});
    leftRow.at(4).front().restrictLowerBound(0, 3, Bounds{-4, false});
    leftRow.at(4).front().restrictUpperBound(0, 3, Bounds{6, false});
    leftRow.at(4).front().restrictLowerBound(1, 1, Bounds{0, false});
    leftRow.at(4).front().restrictUpperBound(1, 1, Bounds{1, false});
    leftRow.at(4).front().restrictLowerBound(1, 2, Bounds{-2, false});
    leftRow.at(4).front().restrictUpperBound(1, 2, Bounds{4, false});
    leftRow.at(4).front().restrictLowerBound(1, 3, Bounds{-2, false});
    leftRow.at(4).front().restrictUpperBound(1, 3, Bounds{4, false});
    leftRow.at(4).front().restrictLowerBound(2, 2, Bounds{-1, false});
    leftRow.at(4).front().restrictUpperBound(2, 2, Bounds{3, false});
    leftRow.at(4).front().restrictLowerBound(2, 3, Bounds{-1, false});
    leftRow.at(4).front().restrictUpperBound(2, 3, Bounds{3, false});
    leftRow.at(4).front().restrictLowerBound(3, 3, Bounds{0, true});
    leftRow.at(4).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, leftRow.at(4).size());
    stream << leftRow.at(4).front();
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 < T_{0, 1}  < 3 && 4 < T_{0, 2}  < 6 && 4 < T_{0, 3}  < 6 && -0 < T_{1, 1}  < 1 && 2 < T_{1, 2}  < 4 && 2 < T_{1, 3}  < 4 && 1 < T_{2, 2}  < 3 && 1 < T_{2, 3}  < 3 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");
    rightRow.emplace_back();
    rightRow.at(4).push_back(TimedCondition{Zone::top(5)});
    rightRow.at(4).front().restrictLowerBound(0, 0, Bounds{-2, false});
    rightRow.at(4).front().restrictUpperBound(0, 0, Bounds{3, false});
    rightRow.at(4).front().restrictLowerBound(0, 1, Bounds{-3, true});
    rightRow.at(4).front().restrictUpperBound(0, 1, Bounds{3, true});
    rightRow.at(4).front().restrictLowerBound(0, 2, Bounds{-4, false});
    rightRow.at(4).front().restrictUpperBound(0, 2, Bounds{6, false});
    rightRow.at(4).front().restrictLowerBound(0, 3, Bounds{-4, false});
    rightRow.at(4).front().restrictUpperBound(0, 3, Bounds{6, false});
    rightRow.at(4).front().restrictLowerBound(1, 1, Bounds{0, false});
    rightRow.at(4).front().restrictUpperBound(1, 1, Bounds{1, false});
    rightRow.at(4).front().restrictLowerBound(1, 2, Bounds{-2, false});
    rightRow.at(4).front().restrictUpperBound(1, 2, Bounds{3, false});
    rightRow.at(4).front().restrictLowerBound(1, 3, Bounds{-2, false});
    rightRow.at(4).front().restrictUpperBound(1, 3, Bounds{3, false});
    rightRow.at(4).front().restrictLowerBound(2, 2, Bounds{-1, false});
    rightRow.at(4).front().restrictUpperBound(2, 2, Bounds{3, false});
    rightRow.at(4).front().restrictLowerBound(2, 3, Bounds{-1, false});
    rightRow.at(4).front().restrictUpperBound(2, 3, Bounds{3, false});
    rightRow.at(4).front().restrictLowerBound(3, 3, Bounds{0, true});
    rightRow.at(4).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, rightRow.at(4).size());
    stream << rightRow.at(4).front();
    BOOST_CHECK_EQUAL("2 < T_{0, 0}  < 3 && 3 <= T_{0, 1}  <= 3 && 4 < T_{0, 2}  < 6 && 4 < T_{0, 3}  < 6 && -0 < T_{1, 1}  < 1 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && 1 < T_{2, 2}  < 3 && 1 < T_{2, 3}  < 3 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");

    suffixes.push_back(suffixes.at(4).predecessor('b'));
    stream << suffixes.at(5);
    BOOST_CHECK_EQUAL("(ba, -0 <= T_{0, 0}  <= 0 && 1 < T_{0, 1}  < 2 && 1 < T_{0, 2}  < 2 && 1 < T_{1, 1}  < 2 && 1 < T_{1, 2}  < 2 && -0 <= T_{2, 2}  <= 0, 0 <= {x0, }{x1, x2, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    leftRow.at(5).push_back(TimedCondition{Zone::top(6)});
    leftRow.at(5).front().restrictLowerBound(0, 0, Bounds{-2, true});
    leftRow.at(5).front().restrictUpperBound(0, 0, Bounds{2, true});
    leftRow.at(5).front().restrictLowerBound(0, 1, Bounds{-2, false});
    leftRow.at(5).front().restrictUpperBound(0, 1, Bounds{3, false});
    leftRow.at(5).front().restrictLowerBound(0, 2, Bounds{-3, false});
    leftRow.at(5).front().restrictUpperBound(0, 2, Bounds{4, false});
    leftRow.at(5).front().restrictLowerBound(0, 3, Bounds{-4, false});
    leftRow.at(5).front().restrictUpperBound(0, 3, Bounds{6, false});
    leftRow.at(5).front().restrictLowerBound(0, 4, Bounds{-4, false});
    leftRow.at(5).front().restrictUpperBound(0, 4, Bounds{6, false});
    leftRow.at(5).front().restrictLowerBound(1, 1, Bounds{0, false});
    leftRow.at(5).front().restrictUpperBound(1, 1, Bounds{1, false});
    leftRow.at(5).front().restrictLowerBound(1, 2, Bounds{-1, false});
    leftRow.at(5).front().restrictUpperBound(1, 2, Bounds{2, false});
    leftRow.at(5).front().restrictLowerBound(1, 3, Bounds{-2, false});
    leftRow.at(5).front().restrictUpperBound(1, 3, Bounds{4, false});
    leftRow.at(5).front().restrictLowerBound(1, 4, Bounds{-2, false});
    leftRow.at(5).front().restrictUpperBound(1, 4, Bounds{4, false});
    leftRow.at(5).front().restrictLowerBound(2, 2, Bounds{-0, false});
    leftRow.at(5).front().restrictUpperBound(2, 2, Bounds{1, false});
    leftRow.at(5).front().restrictLowerBound(2, 3, Bounds{-1, false});
    leftRow.at(5).front().restrictUpperBound(2, 3, Bounds{3, false});
    leftRow.at(5).front().restrictLowerBound(2, 4, Bounds{-1, false});
    leftRow.at(5).front().restrictUpperBound(2, 4, Bounds{3, false});
    leftRow.at(5).front().restrictLowerBound(3, 3, Bounds{-1, false});
    leftRow.at(5).front().restrictUpperBound(3, 3, Bounds{2, false});
    leftRow.at(5).front().restrictLowerBound(3, 4, Bounds{-1, false});
    leftRow.at(5).front().restrictUpperBound(3, 4, Bounds{2, false});
    leftRow.at(5).front().restrictLowerBound(4, 4, Bounds{0, true});
    leftRow.at(5).front().restrictUpperBound(4, 4, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, leftRow.at(5).size());
    stream << leftRow.at(5).front();
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 < T_{0, 1}  < 3 && 3 < T_{0, 2}  < 4 && 4 < T_{0, 3}  < 6 && 4 < T_{0, 4}  < 6 && -0 < T_{1, 1}  < 1 && 1 < T_{1, 2}  < 2 && 2 < T_{1, 3}  < 4 && 2 < T_{1, 4}  < 4 && -0 < T_{2, 2}  < 1 && 1 < T_{2, 3}  < 3 && 1 < T_{2, 4}  < 3 && 1 < T_{3, 3}  < 2 && 1 < T_{3, 4}  < 2 && -0 <= T_{4, 4}  <= 0", stream.str());
    stream.str("");
    rightRow.emplace_back();
    rightRow.at(5).push_back(TimedCondition{Zone::top(6)});
    rightRow.at(5).front().restrictLowerBound(0, 0, Bounds{-2, false});
    rightRow.at(5).front().restrictUpperBound(0, 0, Bounds{3, false});
    rightRow.at(5).front().restrictLowerBound(0, 1, Bounds{-3, true});
    rightRow.at(5).front().restrictUpperBound(0, 1, Bounds{3, true});
    rightRow.at(5).front().restrictLowerBound(0, 2, Bounds{-3, false});
    rightRow.at(5).front().restrictUpperBound(0, 2, Bounds{4, false});
    rightRow.at(5).front().restrictLowerBound(0, 3, Bounds{-4, false});
    rightRow.at(5).front().restrictUpperBound(0, 3, Bounds{6, false});
    rightRow.at(5).front().restrictLowerBound(0, 4, Bounds{-4, false});
    rightRow.at(5).front().restrictUpperBound(0, 4, Bounds{6, false});
    rightRow.at(5).front().restrictLowerBound(1, 1, Bounds{0, false});
    rightRow.at(5).front().restrictUpperBound(1, 1, Bounds{1, false});
    rightRow.at(5).front().restrictLowerBound(1, 2, Bounds{-1, true});
    rightRow.at(5).front().restrictUpperBound(1, 2, Bounds{1, true});
    rightRow.at(5).front().restrictLowerBound(1, 3, Bounds{-2, false});
    rightRow.at(5).front().restrictUpperBound(1, 3, Bounds{3, false});
    rightRow.at(5).front().restrictLowerBound(1, 4, Bounds{-2, false});
    rightRow.at(5).front().restrictUpperBound(1, 4, Bounds{3, false});
    rightRow.at(5).front().restrictLowerBound(2, 2, Bounds{0, false});
    rightRow.at(5).front().restrictUpperBound(2, 2, Bounds{1, false});
    rightRow.at(5).front().restrictLowerBound(2, 3, Bounds{-1, false});
    rightRow.at(5).front().restrictUpperBound(2, 3, Bounds{3, false});
    rightRow.at(5).front().restrictLowerBound(2, 4, Bounds{-1, false});
    rightRow.at(5).front().restrictUpperBound(2, 4, Bounds{3, false});
    rightRow.at(5).front().restrictLowerBound(3, 3, Bounds{-1, false});
    rightRow.at(5).front().restrictUpperBound(3, 3, Bounds{2, false});
    rightRow.at(5).front().restrictLowerBound(3, 4, Bounds{-1, false});
    rightRow.at(5).front().restrictUpperBound(3, 4, Bounds{2, false});
    rightRow.at(5).front().restrictLowerBound(4, 4, Bounds{0, true});
    rightRow.at(5).front().restrictUpperBound(4, 4, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, rightRow.at(5).size());
    stream << rightRow.at(5).front();
    BOOST_CHECK_EQUAL("2 < T_{0, 0}  < 3 && 3 <= T_{0, 1}  <= 3 && 3 < T_{0, 2}  < 4 && 4 < T_{0, 3}  < 6 && 4 < T_{0, 4}  < 6 && -0 < T_{1, 1}  < 1 && 1 <= T_{1, 2}  <= 1 && 2 < T_{1, 3}  < 3 && 2 < T_{1, 4}  < 3 && -0 < T_{2, 2}  < 1 && 1 < T_{2, 3}  < 3 && 1 < T_{2, 4}  < 3 && 1 < T_{3, 3}  < 2 && 1 < T_{3, 4}  < 2 && -0 <= T_{4, 4}  <= 0", stream.str());
    stream.str("");

    suffixes.push_back(suffixes.at(4).predecessor().predecessor());
    stream << suffixes.at(6);
    BOOST_CHECK_EQUAL("(a, 2 < T_{0, 0}  < 3 && 2 < T_{0, 1}  < 3 && -0 <= T_{1, 1}  <= 0, 0 < {x0, x1, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    leftRow.at(6).push_back(TimedCondition{Zone::top(5)});
    leftRow.at(6).front().restrictLowerBound(0, 0, Bounds{-2, true});
    leftRow.at(6).front().restrictUpperBound(0, 0, Bounds{2, true});
    leftRow.at(6).front().restrictLowerBound(0, 1, Bounds{-2, false});
    leftRow.at(6).front().restrictUpperBound(0, 1, Bounds{3, false});
    leftRow.at(6).front().restrictLowerBound(0, 2, Bounds{-5, false});
    leftRow.at(6).front().restrictUpperBound(0, 2, Bounds{6, false});
    leftRow.at(6).front().restrictLowerBound(0, 3, Bounds{-5, false});
    leftRow.at(6).front().restrictUpperBound(0, 3, Bounds{6, false});
    leftRow.at(6).front().restrictLowerBound(1, 1, Bounds{0, false});
    leftRow.at(6).front().restrictUpperBound(1, 1, Bounds{1, false});
    leftRow.at(6).front().restrictLowerBound(1, 2, Bounds{-3, false});
    leftRow.at(6).front().restrictUpperBound(1, 2, Bounds{4, false});
    leftRow.at(6).front().restrictLowerBound(1, 3, Bounds{-3, false});
    leftRow.at(6).front().restrictUpperBound(1, 3, Bounds{4, false});
    leftRow.at(6).front().restrictLowerBound(2, 2, Bounds{-2, false});
    leftRow.at(6).front().restrictUpperBound(2, 2, Bounds{3, false});
    leftRow.at(6).front().restrictLowerBound(2, 3, Bounds{-2, false});
    leftRow.at(6).front().restrictUpperBound(2, 3, Bounds{3, false});
    leftRow.at(6).front().restrictLowerBound(3, 3, Bounds{0, true});
    leftRow.at(6).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, leftRow.at(6).size());
    stream << leftRow.at(6).front();
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 < T_{0, 1}  < 3 && 5 < T_{0, 2}  < 6 && 5 < T_{0, 3}  < 6 && -0 < T_{1, 1}  < 1 && 3 < T_{1, 2}  < 4 && 3 < T_{1, 3}  < 4 && 2 < T_{2, 2}  < 3 && 2 < T_{2, 3}  < 3 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");
    rightRow.emplace_back();
    rightRow.at(6).push_back(TimedCondition{Zone::top(5)});
    rightRow.at(6).front().restrictLowerBound(0, 0, Bounds{-2, false});
    rightRow.at(6).front().restrictUpperBound(0, 0, Bounds{3, false});
    rightRow.at(6).front().restrictLowerBound(0, 1, Bounds{-3, true});
    rightRow.at(6).front().restrictUpperBound(0, 1, Bounds{3, true});
    rightRow.at(6).front().restrictLowerBound(0, 2, Bounds{-5, false});
    rightRow.at(6).front().restrictUpperBound(0, 2, Bounds{6, false});
    rightRow.at(6).front().restrictLowerBound(0, 3, Bounds{-5, false});
    rightRow.at(6).front().restrictUpperBound(0, 3, Bounds{6, false});
    rightRow.at(6).front().restrictLowerBound(1, 1, Bounds{0, false});
    rightRow.at(6).front().restrictUpperBound(1, 1, Bounds{1, false});
    rightRow.at(6).front().restrictLowerBound(1, 2, Bounds{-3, false});
    rightRow.at(6).front().restrictUpperBound(1, 2, Bounds{4, false});
    rightRow.at(6).front().restrictLowerBound(1, 3, Bounds{-3, false});
    rightRow.at(6).front().restrictUpperBound(1, 3, Bounds{4, false});
    rightRow.at(6).front().restrictLowerBound(2, 2, Bounds{-2, false});
    rightRow.at(6).front().restrictUpperBound(2, 2, Bounds{3, false});
    rightRow.at(6).front().restrictLowerBound(2, 3, Bounds{-2, false});
    rightRow.at(6).front().restrictUpperBound(2, 3, Bounds{3, false});
    rightRow.at(6).front().restrictLowerBound(3, 3, Bounds{0, true});
    rightRow.at(6).front().restrictUpperBound(3, 3, Bounds{0, true});
    BOOST_CHECK_EQUAL(1, rightRow.at(6).size());
    stream << rightRow.at(6).front();
    BOOST_CHECK_EQUAL("2 < T_{0, 0}  < 3 && 3 <= T_{0, 1}  <= 3 && 5 < T_{0, 2}  < 6 && 5 < T_{0, 3}  < 6 && -0 < T_{1, 1}  < 1 && 3 < T_{1, 2}  < 4 && 3 < T_{1, 3}  < 4 && 2 < T_{2, 2}  < 3 && 2 < T_{2, 3}  < 3 && -0 <= T_{3, 3}  <= 0", stream.str());
    stream.str("");

    suffixes.push_back(suffixes.at(0).predecessor());
    stream << suffixes.at(7);
    BOOST_CHECK_EQUAL("(, -0 < T_{0, 0}  < 1, 0 < {x0, }", stream.str());
    stream.str("");
    leftRow.emplace_back();
    rightRow.emplace_back();
    BOOST_TEST(leftRow.at(7).empty());
    BOOST_TEST(rightRow.at(7).empty());

    // The actual testing
    auto renamingRelOpt = findEquivalentRenaming(left, leftRow, right, rightRow, suffixes);
    BOOST_TEST(renamingRelOpt.has_value());
    BOOST_TEST(equivalence(left, leftRow, right, rightRow, suffixes, renamingRelation));
    {
      auto juxtaposition = left.getTimedCondition() ^ right.getTimedCondition();
      juxtaposition.addRenaming(renamingRelation);
      BOOST_TEST(bool(juxtaposition));
      const auto leftConcatenation = left + suffixes.at(4);
      stream << leftConcatenation;
      BOOST_CHECK_EQUAL("(bba, 2 <= T_{0, 0}  <= 2 && 2 < T_{0, 1}  < 3 && 4 < T_{0, 2}  < 6 && 4 < T_{0, 3}  < 6 && -0 < T_{1, 1}  < 1 && 2 < T_{1, 2}  < 4 && 2 < T_{1, 3}  < 4 && 1 < T_{2, 2}  < 3 && 1 < T_{2, 3}  < 3 && -0 <= T_{3, 3}  <= 0)", stream.str());
      stream.str("");
      const auto rightConcatenation = right + suffixes.at(4);
      stream << rightConcatenation;
      BOOST_CHECK_EQUAL("(bba, 2 < T_{0, 0}  < 3 && 3 <= T_{0, 1}  <= 3 && 4 < T_{0, 2}  < 6 && 4 < T_{0, 3}  < 6 && -0 < T_{1, 1}  < 1 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && 1 < T_{2, 2}  < 3 && 1 < T_{2, 3}  < 3 && -0 <= T_{3, 3}  <= 0)", stream.str());
      stream.str("");

      auto leftJuxtaposition = JuxtaposedZoneSet{leftRow.at(4),
                                                 rightConcatenation.getTimedCondition(),
                                                 suffixes.at(4).wordSize()};
      auto rightJuxtaposition = JuxtaposedZoneSet{leftConcatenation.getTimedCondition(),
                                                  rightRow.at(4),
                                                  suffixes.at(4).wordSize()};
      BOOST_CHECK_EQUAL(leftJuxtaposition, rightJuxtaposition);
      leftJuxtaposition.addRenaming(renamingRelation);
      rightJuxtaposition.addRenaming(renamingRelation);
      BOOST_CHECK_EQUAL(leftJuxtaposition, rightJuxtaposition);
    }
  }

BOOST_AUTO_TEST_SUITE_END()

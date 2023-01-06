/**
 * @author Masaki Waga
 * @date 2022/12/30.
 */
#include <sstream>
#include <boost/test/unit_test.hpp>

#define private public

#include "neighbor_conditions.hh"

BOOST_AUTO_TEST_SUITE(NeighborConditionsTest)
  using namespace learnta;
  struct NeighborConditionsFixture {
    ForwardRegionalElementaryLanguage elementary;
    std::unordered_set<ClockVariables> preciseClocks = {1};
    NeighborConditions neighborConditions;
    NeighborConditionsFixture() : elementary(ForwardRegionalElementaryLanguage().successor().successor().successor().successor().successor('a').successor('b').successor().successor().successor().successor().successor()),
                                  neighborConditions(elementary, preciseClocks) {}
  };

  struct NeighborConditionsFixture20221231 {
    ForwardRegionalElementaryLanguage elementary;
    std::unordered_set<ClockVariables> preciseClocks = {1, 2};
    NeighborConditions neighborConditions;
    NeighborConditionsFixture20221231() : elementary(ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"ab", {2.0, 0, 2.5}})),
                                          neighborConditions(elementary, preciseClocks) {}
  };

  // [2023-01-06 15:32:59.182816] [0x0000000116a57600] [debug]   applying: prefix: 2.75 b 0 suffix: 0.25 a 0
  // morphism: domain: (b, 2 < T_{0, 0}  < 3 && 2 < T_{0, 1}  < 3 && -0 <= T_{1, 1}  <= 0)
  // codomain: (abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0) renaming: {t0 == t'1}
  struct NeighborConditionsFixture20230106 {
    ForwardRegionalElementaryLanguage elementary;
    std::unordered_set<ClockVariables> preciseClocks = {1};
    std::unordered_set<ClockVariables> explicitPreciseClocks = {1, 2, 3};
    NeighborConditions neighborConditions;
    NeighborConditionsFixture20230106() : elementary(ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"abb", {1.0, 2.75, 0, 0}})),
                                          neighborConditions(elementary, preciseClocks) {}
  };

  BOOST_FIXTURE_TEST_CASE(fixtureTest, NeighborConditionsFixture) {
    std::stringstream stream;
    stream << static_cast<ElementaryLanguage>(elementary);
    BOOST_CHECK_EQUAL("(ab, 2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 4 < T_{0, 2}  < 5 && -0 <= T_{1, 1}  <= 0 && 2 < T_{1, 2}  < 3 && 2 < T_{2, 2}  < 3)",
                      stream.str());
    stream.str("");
  }

  BOOST_FIXTURE_TEST_CASE(fixtureTest20221231, NeighborConditionsFixture20221231) {
    std::stringstream stream;
    stream << static_cast<ElementaryLanguage>(elementary);
    BOOST_CHECK_EQUAL("(ab, 2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 4 < T_{0, 2}  < 5 && 0 <= T_{1, 1}  <= 0 && 2 < T_{1, 2}  < 3 && 2 < T_{2, 2}  < 3)",
                      stream.str());
    stream.str("");
  }

  BOOST_FIXTURE_TEST_CASE(fixtureTest20230106, NeighborConditionsFixture20230106) {
    std::stringstream stream;
    stream << static_cast<ElementaryLanguage>(elementary);
    BOOST_CHECK_EQUAL("(abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && 0 <= T_{2, 2}  <= 0 && 0 <= T_{2, 3}  <= 0 && 0 <= T_{3, 3}  <= 0)",
                      stream.str());
    stream.str("");
  }

  BOOST_FIXTURE_TEST_CASE(constructionTest, NeighborConditionsFixture) {
    std::stringstream stream;
    // Test the clock size
    BOOST_TEST(neighborConditions.clockSize == 3);
    // Test the neighbor size
    BOOST_TEST(neighborConditions.neighbors.size() == 2);
    // 2 < T_{0, 0}  < 3 is impossible because it requires -1 < T_{1, 1} < 0
    std::vector<std::string> expectedNeighbors = {
            "(ab, 1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 4 < T_{0, 2}  < 5 && 0 < T_{1, 1}  < 1 && 2 < T_{1, 2}  < 3 && 2 < T_{2, 2}  < 3, 0 < {x0, x2, }{x1, })",
            "(ab, 2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 4 < T_{0, 2}  < 5 && 0 <= T_{1, 1}  <= 0 && 2 < T_{1, 2}  < 3 && 2 < T_{2, 2}  < 3, 0 < {x0, x1, x2, })"
    };
    std::sort(expectedNeighbors.begin(), expectedNeighbors.end());
    std::vector<std::string> neighborsString;
    std::transform(neighborConditions.neighbors.begin(), neighborConditions.neighbors.end(), std::back_inserter(neighborsString), [] (const auto& condition) {
      std::stringstream stream;
      stream << condition;
      return stream.str();
    });
    std::sort(neighborsString.begin(), neighborsString.end());
    BOOST_TEST(expectedNeighbors == neighborsString, boost::test_tools::per_element());
  }

  BOOST_FIXTURE_TEST_CASE(discreteSuccessorTest, NeighborConditionsFixture) {
    const auto successor = neighborConditions.successor('a');
    // The clock size increases by one
    BOOST_TEST(successor.clockSize == neighborConditions.clockSize + 1);
    // The neighbor size does not change by discrete successors
    BOOST_TEST(successor.neighbors.size() == neighborConditions.neighbors.size());
    // The precise clock increases by one
    BOOST_TEST(successor.preciseClocks.size() == neighborConditions.preciseClocks.size() + 1);
  }

  BOOST_FIXTURE_TEST_CASE(continuousSuccessorTest, NeighborConditionsFixture) {
    const auto successor = neighborConditions.successor();
    // The clock size does not change
    BOOST_TEST(successor.clockSize == neighborConditions.clockSize);
    // The precise clock does not change
    BOOST_TEST(successor.preciseClocks == neighborConditions.preciseClocks, boost::test_tools::per_element());
    std::vector<std::string> expectedNeighbors = {
            "(ab, 1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 4 < T_{0, 2}  < 5 && 0 < T_{1, 1}  < 1 && 3 <= T_{1, 2}  <= 3 && 2 < T_{2, 2}  < 3, 0 <= {x1, }{x0, x2, })",
            "(ab, 2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 5 <= T_{0, 2}  <= 5 && 0 <= T_{1, 1}  <= 0 && 3 <= T_{1, 2}  <= 3 && 3 <= T_{2, 2}  <= 3, 0 <= {x0, x1, x2, })"
    };
    std::sort(expectedNeighbors.begin(), expectedNeighbors.end());
    std::vector<std::string> neighborsString;
    std::transform(successor.neighbors.begin(), successor.neighbors.end(), std::back_inserter(neighborsString), [] (const auto& condition) {
      std::stringstream stream;
      stream << condition;
      return stream.str();
    });
    std::sort(neighborsString.begin(), neighborsString.end());
    BOOST_TEST(expectedNeighbors == neighborsString, boost::test_tools::per_element());
  }

  BOOST_FIXTURE_TEST_CASE(continuousSuccessorTest20221231, NeighborConditionsFixture20221231) {
    const auto successor = neighborConditions.successor();
    // The clock size does not change
    BOOST_TEST(successor.clockSize == neighborConditions.clockSize);
    // The precise clock does not change
    BOOST_TEST(successor.preciseClocks == neighborConditions.preciseClocks);
    std::vector<std::string> expectedNeighbors = {
            "(ab, 1 < T_{0, 0}  < 2 && 1 < T_{0, 1}  < 2 && 4 < T_{0, 2}  < 5 && 0 <= T_{1, 1}  <= 0 && 3 <= T_{1, 2}  <= 3 && 3 <= T_{2, 2}  <= 3, 0 <= {x1, x2, }{x0, })",
            "(ab, 2 < T_{0, 0}  < 3 && 2 < T_{0, 1}  < 3 && 5 < T_{0, 2}  < 6 && 0 <= T_{1, 1}  <= 0 && 3 <= T_{1, 2}  <= 3 && 3 <= T_{2, 2}  <= 3, 0 <= {x1, x2, }{x0, })",
            "(ab, 2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 5 <= T_{0, 2}  <= 5 && 0 <= T_{1, 1}  <= 0 && 3 <= T_{1, 2}  <= 3 && 3 <= T_{2, 2}  <= 3, 0 <= {x0, x1, x2, })"
    };
    std::sort(expectedNeighbors.begin(), expectedNeighbors.end());
    std::vector<std::string> neighborsString;
    std::transform(successor.neighbors.begin(), successor.neighbors.end(), std::back_inserter(neighborsString), [] (const auto& condition) {
      std::stringstream stream;
      stream << condition;
      return stream.str();
    });
    std::sort(neighborsString.begin(), neighborsString.end());
    BOOST_TEST(expectedNeighbors == neighborsString, boost::test_tools::per_element());
  }

  BOOST_FIXTURE_TEST_CASE(toRelaxedGuardTest, NeighborConditionsFixture) {
    const auto sort = [] (std::vector<Constraint> guard) -> std::vector<Constraint> {
      std::sort(guard.begin(), guard.end(), [] (const Constraint &left, const Constraint &right) -> bool {
        return std::make_pair(left.x, left.toDBMBound()) <= std::make_pair(right.x, right.toDBMBound());
      });
      return guard;
    };
    // If we do not take successor, the guard is the same as the original one.
    BOOST_TEST(sort(neighborConditions.toOriginalGuard()) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());
    std::vector<Constraint> expectedSuccessorGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) <= 5,
            ConstraintMaker(1) <= 3, ConstraintMaker(1) >= 3,
            ConstraintMaker(2) > 2, ConstraintMaker(2) <= 3
    };
    BOOST_TEST(sort(expectedSuccessorGuard) == sort(neighborConditions.successor().toRelaxedGuard()),
               boost::test_tools::per_element());
  }

  BOOST_FIXTURE_TEST_CASE(toRelaxedGuardTest20221231, NeighborConditionsFixture20221231) {
    const auto sort = [] (std::vector<Constraint> guard) -> std::vector<Constraint> {
      std::sort(guard.begin(), guard.end(), [] (const Constraint &left, const Constraint &right) -> bool {
        return std::make_pair(left.x, left.toDBMBound()) <= std::make_pair(right.x, right.toDBMBound());
      });
      return guard;
    };
    // If we do not take successor, the guard is the same as the original one.
    BOOST_TEST(sort(neighborConditions.toOriginalGuard()) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());
    std::vector<Constraint> expectedSuccessorGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) < 6,
            ConstraintMaker(1) <= 3, ConstraintMaker(1) >= 3,
            ConstraintMaker(2) >= 3, ConstraintMaker(2) <= 3
    };
    BOOST_TEST(sort(expectedSuccessorGuard) == sort(neighborConditions.successor().toRelaxedGuard()),
               boost::test_tools::per_element());
  }

  BOOST_FIXTURE_TEST_CASE(toRelaxedGuardTest20230106, NeighborConditionsFixture20230106) {
    const auto sort = [] (std::vector<Constraint> guard) -> std::vector<Constraint> {
      std::sort(guard.begin(), guard.end(), [] (const Constraint &left, const Constraint &right) -> bool {
        return std::make_pair(left.x, left.toDBMBound()) <= std::make_pair(right.x, right.toDBMBound());
      });
      return guard;
    };
    // If we do not take successor, the guard is the same as the original one.
    BOOST_TEST(sort(neighborConditions.toOriginalGuard()) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());

    std::vector<Constraint> expectedSuccessorGuard = {
            ConstraintMaker(0) > 3, ConstraintMaker(0) < 5,
            ConstraintMaker(1) > 2, ConstraintMaker(1) < 3,
            ConstraintMaker(2) > 0, ConstraintMaker(2) < 1,
            ConstraintMaker(3) > 0, ConstraintMaker(3) < 1
    };
    neighborConditions.successorAssign();
    BOOST_TEST(sort(expectedSuccessorGuard) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());

    expectedSuccessorGuard = {
            ConstraintMaker(0) > 3, ConstraintMaker(0) < 5,
            ConstraintMaker(1) >= 3, ConstraintMaker(1) <= 3,
            ConstraintMaker(2) > 0, ConstraintMaker(2) < 1,
            ConstraintMaker(3) > 0, ConstraintMaker(3) < 1
    };
    neighborConditions.successorAssign();
    BOOST_TEST(sort(expectedSuccessorGuard) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());

    expectedSuccessorGuard = {
            ConstraintMaker(0) > 3, ConstraintMaker(0) < 5,
            ConstraintMaker(1) > 3, ConstraintMaker(1) < 4,
            ConstraintMaker(2) > 0, ConstraintMaker(2) < 1,
            ConstraintMaker(3) > 0, ConstraintMaker(3) < 1
    };
    neighborConditions.successorAssign();
    BOOST_TEST(sort(expectedSuccessorGuard) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());

    expectedSuccessorGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) < 5,
            ConstraintMaker(1) > 3, ConstraintMaker(1) < 4,
            ConstraintMaker(2) >= 1, ConstraintMaker(2) <= 1,
            ConstraintMaker(3) >= 1, ConstraintMaker(3) <= 1
    };
    neighborConditions.successorAssign();
    BOOST_TEST(sort(expectedSuccessorGuard) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());
  }
BOOST_AUTO_TEST_SUITE_END()
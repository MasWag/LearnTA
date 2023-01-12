/**
 * @author Masaki Waga
 * @date 2022/12/30.
 */
#include <sstream>
#include <boost/test/unit_test.hpp>

#define private public

#include "neighbor_conditions.hh"
#include "simple_automaton_fixture.hh"

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

  // [2023-01-07 23:29:49.923297] [0x000000010ed7d600] [debug]   current imprecise neighbors: 0x600002c801f8, (abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 4 < T_{0, 3}  < 5 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 3 < T_{1, 3}  < 4 && -0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x2, x3, }{x0, x1, }) {x0, x1} {
  //(abb, 1 <= T_{0, 0}  <= 1 && 2 < T_{0, 1}  < 3 && 2 < T_{0, 2}  < 3 && 4 < T_{0, 3}  < 5 && 1 < T_{1, 1}  < 2 && 1 < T_{1, 2}  < 2 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x0, x1, }{x2, x3, })
  //(abb, 1 <= T_{0, 0}  <= 1 && 3 <= T_{0, 1}  <= 3 && 3 <= T_{0, 2}  <= 3 && 4 < T_{0, 3}  < 5 && 2 <= T_{1, 1}  <= 2 && 2 <= T_{1, 2}  <= 2 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x0, x1, x2, x3, })
  //(abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 4 < T_{0, 3}  < 5 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x2, x3, }{x0, x1, })
  //}
  // (abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 4 < T_{0, 3}  < 5 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 3 < T_{1, 3}  < 4 && -0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x2, x3, }{x0, x1, }), {t0 == t'0 && t1 == t'1}
  struct NeighborConditionsFixture20230108 {
    ForwardRegionalElementaryLanguage elementary;
    std::unordered_set<ClockVariables> preciseClocks = {0, 1};
    NeighborConditions neighborConditions;
    NeighborConditionsFixture20230108() : elementary(ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"abb", {1.0, 2.5, 0, 1.25}})),
                                          neighborConditions(elementary, preciseClocks) {}
  };

  // (b, -0 < T_{0, 0}  < 1 && 6 < T_{0, 1}  < 7 && 6 < T_{1, 1}  < 7, 0 < {x1, }{x0, }) {x1}
  struct NeighborConditionsFixture20230109 {
    ForwardRegionalElementaryLanguage elementary;
    std::unordered_set<ClockVariables> preciseClocks = {1};
    NeighborConditions neighborConditions;
    NeighborConditionsFixture20230109() : elementary(ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"b", {0.25, 6.5}})),
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

  BOOST_FIXTURE_TEST_CASE(fixtureTest20230108, NeighborConditionsFixture20230108) {
    std::stringstream stream;
    stream << elementary;
    BOOST_CHECK_EQUAL("(abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 4 < T_{0, 3}  < 5 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x2, x3, }{x0, x1, })",
                      stream.str());
    stream.str("");
  }

  BOOST_FIXTURE_TEST_CASE(fixtureTest20230109, NeighborConditionsFixture20230109) {
    std::stringstream stream;
    stream << elementary;
    BOOST_CHECK_EQUAL("(b, 0 < T_{0, 0}  < 1 && 6 < T_{0, 1}  < 7 && 6 < T_{1, 1}  < 7, 0 < {x1, }{x0, })",
                      stream.str());
    stream.str("");
  }

  BOOST_FIXTURE_TEST_CASE(constructionTest, NeighborConditionsFixture) {
    std::stringstream stream;
    // Test the clock size
    BOOST_TEST(neighborConditions.clockSize == 3);
    // Test the neighbor size
    BOOST_TEST(neighborConditions.neighbors.size() == 5);
    std::vector<std::string> expectedNeighbors = {
            "(ab, 2 < T_{0, 0}  < 3 && 2 <= T_{0, 1}  <= 2 && 4 < T_{0, 2}  < 5 && -1 < T_{1, 1}  < 0 && 2 < T_{1, 2}  < 3 && 2 < T_{2, 2}  < 3, 0 < {x1, }{x0, x2, })",
            "(ab, 2 < T_{0, 0}  < 3 && 2 <= T_{0, 1}  <= 2 && 5 < T_{0, 2}  < 6 && -1 < T_{1, 1}  < 0 && 2 < T_{1, 2}  < 3 && 3 < T_{2, 2}  < 4, 0 < {x0, x2, }{x1, })",
            "(ab, 2 < T_{0, 0}  < 3 && 2 <= T_{0, 1}  <= 2 && 5 <= T_{0, 2}  <= 5 && -1 < T_{1, 1}  < 0 && 2 < T_{1, 2}  < 3 && 3 <= T_{2, 2}  <= 3, 0 <= {x0, x2, }{x1, })",
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

  BOOST_FIXTURE_TEST_CASE(constructionTest20230108, NeighborConditionsFixture20230108) {
    std::stringstream stream;
    // Test the clock size
    BOOST_TEST(neighborConditions.clockSize == 4);
    // Test the neighbor size
    BOOST_TEST(neighborConditions.neighbors.size() == 5);
    std::vector<std::string> expectedNeighbors = {
            "(abb, 1 <= T_{0, 0}  <= 1 && 2 < T_{0, 1}  < 3 && 2 < T_{0, 2}  < 3 && 4 < T_{0, 3}  < 5 && 1 < T_{1, 1}  < 2 && 1 < T_{1, 2}  < 2 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x0, x1, }{x2, x3, })",
            "(abb, 1 <= T_{0, 0}  <= 1 && 3 <= T_{0, 1}  <= 3 && 3 <= T_{0, 2}  <= 3 && 4 < T_{0, 3}  < 5 && 2 <= T_{1, 1}  <= 2 && 2 <= T_{1, 2}  <= 2 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x0, x1, x2, x3, })",
            "(abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 4 < T_{0, 3}  < 5 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 1 < T_{2, 3}  < 2 && 1 < T_{3, 3}  < 2, 0 < {x2, x3, }{x0, x1, })",
            // The following are added by taking successors for imprecise clocks
            "(abb, 1 <= T_{0, 0}  <= 1 && 2 < T_{0, 1}  < 3 && 2 < T_{0, 2}  < 3 && 4 < T_{0, 3}  < 5 && 1 < T_{1, 1}  < 2 && 1 < T_{1, 2}  < 2 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 2 <= T_{2, 3}  <= 2 && 2 <= T_{3, 3}  <= 2, 0 <= {x2, x3, }{x0, x1, })",
            "(abb, 1 <= T_{0, 0}  <= 1 && 2 < T_{0, 1}  < 3 && 2 < T_{0, 2}  < 3 && 4 < T_{0, 3}  < 5 && 1 < T_{1, 1}  < 2 && 1 < T_{1, 2}  < 2 && 3 < T_{1, 3}  < 4 && 0 <= T_{2, 2}  <= 0 && 2 < T_{2, 3}  < 3 && 2 < T_{3, 3}  < 3, 0 < {x2, x3, }{x0, x1, })"
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

  BOOST_FIXTURE_TEST_CASE(constructionTest20230109, NeighborConditionsFixture20230109) {
    std::stringstream stream;
    // Test the clock size
    BOOST_TEST(neighborConditions.clockSize == 2);
    // Test the neighbor size
    BOOST_TEST(neighborConditions.neighbors.size() == 5);
    std::vector<std::string> expectedNeighbors = {
            "(b, -1 < T_{0, 0}  < 0 && 6 < T_{0, 1}  < 7 && 6 < T_{1, 1}  < 7, 0 < {x0, }{x1, })",
            "(b, 0 <= T_{0, 0}  <= 0 && 6 < T_{0, 1}  < 7 && 6 < T_{1, 1}  < 7, 0 < {x0, x1, })",
            "(b, 0 < T_{0, 0}  < 1 && 7 <= T_{0, 1}  <= 7 && 6 < T_{1, 1}  < 7, 0 <= {x0, }{x1, })",
            "(b, 0 < T_{0, 0}  < 1 && 6 < T_{0, 1}  < 7 && 6 < T_{1, 1}  < 7, 0 < {x1, }{x0, })",
            "(b, 0 < T_{0, 0}  < 1 && 7 < T_{0, 1}  < 8 && 6 < T_{1, 1}  < 7, 0 < {x0, }{x1, })"
    };
    std::sort(expectedNeighbors.begin(), expectedNeighbors.end());
    std::vector<std::string> neighborsString;
    std::transform(neighborConditions.neighbors.begin(), neighborConditions.neighbors.end(),
                   std::back_inserter(neighborsString), [] (const auto& condition) {
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
            "(ab, 2 < T_{0, 0}  < 3 && 2 <= T_{0, 1}  <= 2 && 5 < T_{0, 2}  < 6 && -1 < T_{1, 1}  < 0 && 3 <= T_{1, 2}  <= 3 && 3 < T_{2, 2}  < 4, 0 <= {x1, }{x0, x2, })",
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
    std::vector<Constraint> expectedOriginalGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) < 5,
            ConstraintMaker(1) < 3, ConstraintMaker(1) > 2,
            ConstraintMaker(2) > 2, ConstraintMaker(2) < 3
    };
    BOOST_TEST(sort(expectedOriginalGuard) == sort(neighborConditions.toOriginalGuard()),
               boost::test_tools::per_element());
    std::vector<Constraint> expectedRelaxedGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) < 6,
            ConstraintMaker(1) < 3, ConstraintMaker(1) > 2,
            ConstraintMaker(2) > 2, ConstraintMaker(2) < 4
    };
    BOOST_TEST(sort(expectedRelaxedGuard) == sort(neighborConditions.toRelaxedGuard()),
               boost::test_tools::per_element());
    std::vector<Constraint> expectedSuccessorGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) < 6,
            ConstraintMaker(1) <= 3, ConstraintMaker(1) >= 3,
            ConstraintMaker(2) > 2, ConstraintMaker(2) < 4
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
    std::vector<Constraint> expectedOriginalGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) < 5,
            ConstraintMaker(1) < 3, ConstraintMaker(1) > 2,
            ConstraintMaker(2) > 2, ConstraintMaker(2) < 3
    };
    BOOST_TEST(sort(expectedOriginalGuard) == sort(neighborConditions.toOriginalGuard()),
               boost::test_tools::per_element());
    std::vector<Constraint> expectedRelaxedGuard = {
            ConstraintMaker(0) > 4, ConstraintMaker(0) < 6,
            ConstraintMaker(1) < 3, ConstraintMaker(1) > 2,
            ConstraintMaker(2) > 2, ConstraintMaker(2) < 3
    };
    BOOST_TEST(sort(expectedRelaxedGuard) == sort(neighborConditions.toRelaxedGuard()),
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

  BOOST_FIXTURE_TEST_CASE(matchAndRelax20230106, NeighborConditionsFixture20230106) {
    //         loc6->loc1 [label="b", guard="{x0 >= 4, x0 <= 4, x1 >= 3, x1 <= 3, x2 > 0, x2 < 1, x3 > 0, x3 < 1}", reset="{x0 := 8, x1 := 7}"] is supposed to be relaxed, but not :-(
    TATransition::Resets resets;
    resets.emplace_back(static_cast<ClockVariables>(0), static_cast<ClockVariables>(8));
    resets.emplace_back(static_cast<ClockVariables>(1), static_cast<ClockVariables>(7));
    const TATransition transition {new TAState(),
                                   resets,
                                   std::vector<Constraint> {ConstraintMaker(0) >= 4, ConstraintMaker(0) <= 4,
                                                            ConstraintMaker(1) >= 3, ConstraintMaker(1) <= 3,
                                                            ConstraintMaker(2) > 0, ConstraintMaker(2) < 1,
                                                            ConstraintMaker(3) > 0, ConstraintMaker(3) < 1}};
    BOOST_TEST(!neighborConditions.match(transition));
    neighborConditions.successorAssign();
    BOOST_TEST(!neighborConditions.match(transition));
    neighborConditions.successorAssign();
    BOOST_TEST(neighborConditions.match(transition));
    auto relaxedGuard = neighborConditions.toRelaxedGuard();
    BOOST_TEST(isWeaker(relaxedGuard, transition.guard));
    BOOST_TEST(!isWeaker(transition.guard, relaxedGuard));
  }

  BOOST_AUTO_TEST_CASE(preciseClocksAfterResetTest) {
    std::unordered_set<ClockVariables> preciseClocks = {2};
    TATransition::Resets resets;
    resets.emplace_back(1, 5.75);
    resets.emplace_back(3, 0.0);
    std::unordered_set<ClockVariables> expected = {2, 3};
    BOOST_TEST(expected == NeighborConditions::preciseClocksAfterReset(preciseClocks, resets));
    resets.clear();
    resets.emplace_back(0, 8.0);
    resets.emplace_back(1, 1.5);
    resets.emplace_back(2, 0.0);
    std::unordered_set<ClockVariables> expectedSecond = {0, 2, 3};
    BOOST_TEST(expectedSecond == NeighborConditions::preciseClocksAfterReset(expected, resets));
  }

  BOOST_FIXTURE_TEST_CASE(computeTargetClockSizeTest, SimpleDTALearnedFixture) {
    this->validate();
    std::unordered_map<TAState*, std::size_t> expectedClockSize;
    expectedClockSize[hypothesis.states.at(0).get()] = 1;
    expectedClockSize[hypothesis.states.at(1).get()] = 2;

    for (const auto &state: hypothesis.states) {
      for (const auto &[action, transitions]: state->next) {
        for (const auto &transition: transitions) {
          BOOST_CHECK_EQUAL(expectedClockSize.at(transition.target),
                            NeighborConditions::computeTargetClockSize(transition));
        }
      }
    }
  }
BOOST_AUTO_TEST_SUITE_END()
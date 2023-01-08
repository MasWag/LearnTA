/**
 * @author Masaki Waga
 * @date 2022/12/22.
 */

#include <boost/test/unit_test.hpp>
#include <sstream>
#define protected public
#include "external_transition_maker.hh"

BOOST_AUTO_TEST_SUITE(ExternalTransitionMakerTest)
  using namespace learnta;

  BOOST_AUTO_TEST_CASE(makeWithImplicitEquivalenceTest) {
    std::stringstream stream;
    // domain: (ab, 1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 1 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1)
    TimedCondition domain {Zone::top(4)};
    domain.restrictLowerBound(0, 0, Bounds{-1, false});
    domain.restrictUpperBound(0, 0, Bounds{2, false});
    domain.restrictLowerBound(0, 1, Bounds{-2, true});
    domain.restrictUpperBound(0, 1, Bounds{2, true});
    domain.restrictLowerBound(0, 2, Bounds{-2, false});
    domain.restrictUpperBound(0, 2, Bounds{3, false});
    domain.restrictLowerBound(1, 1, Bounds{0, false});
    domain.restrictUpperBound(1, 1, Bounds{1, false});
    domain.restrictLowerBound(1, 2, Bounds{0, false});
    domain.restrictUpperBound(1, 2, Bounds{1, false});
    domain.restrictLowerBound(2, 2, Bounds{0, false});
    domain.restrictUpperBound(2, 2, Bounds{1, false});
    stream << domain;
    BOOST_CHECK_EQUAL("1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 1 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1", stream.str());
    stream.str("");

    // codomain: (ab, 2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 <= T_{1, 1}  <= 0 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1)
    TimedCondition codomain {Zone::top(4)};
    codomain.restrictLowerBound(0, 0, Bounds{-2, true});
    codomain.restrictUpperBound(0, 0, Bounds{2, true});
    codomain.restrictLowerBound(0, 1, Bounds{-2, true});
    codomain.restrictUpperBound(0, 1, Bounds{2, true});
    codomain.restrictLowerBound(0, 2, Bounds{-2, false});
    codomain.restrictUpperBound(0, 2, Bounds{3, false});
    codomain.restrictLowerBound(1, 1, Bounds{0, true});
    codomain.restrictUpperBound(1, 1, Bounds{0, true});
    codomain.restrictLowerBound(1, 2, Bounds{0, false});
    codomain.restrictUpperBound(1, 2, Bounds{1, false});
    codomain.restrictLowerBound(2, 2, Bounds{0, false});
    codomain.restrictUpperBound(2, 2, Bounds{1, false});
    stream << codomain;
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 <= T_{1, 1}  <= 0 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1", stream.str());
    stream.str("");

    // renaming: {t0 == t'0}
    RenamingRelation renaming;
    renaming.emplace_back(0, 0);
    stream << renaming;
    BOOST_CHECK_EQUAL("{t0 == t'0}", stream.str());
    stream.str("");

    ExternalTransitionMaker maker;
    maker.add(std::make_shared<TAState>(false), renaming, domain, codomain);
    const auto transitions = maker.make();
    BOOST_CHECK_EQUAL(1, transitions.size());
    const std::vector<Constraint> expectedGuard = {ConstraintMaker(0) > 2,
                                                   ConstraintMaker(0) < 3,
                                                   ConstraintMaker(1) > 0,
                                                   ConstraintMaker(1) < 1,
                                                   ConstraintMaker(2) > 0,
                                                   ConstraintMaker(2) < 1};
    BOOST_TEST(expectedGuard == transitions.front().guard, boost::test_tools::per_element());
    TATransition::Resets expectedReset;
    expectedReset.emplace_back(1, static_cast<ClockVariables>(2));
    BOOST_TEST(expectedReset == transitions.front().resetVars, boost::test_tools::per_element());
  }

  BOOST_AUTO_TEST_CASE(inactiveClockVariablesTest) {
    std::stringstream stream;
    // renaming: {}
    RenamingRelation renaming;
    // timed condition: 2 < T_{0, 0}  < 3 && 4 <= T_{0, 1}  <= 4 && 4 <= T_{0, 2}  <= 4 && 1 < T_{1, 1}  < 2 && 1 < T_{1, 2}  < 2 && -0 <= T_{2, 2}  <= 0
    TimedCondition condition {Zone::top(4)};
    condition.restrictLowerBound(0, 0, Bounds{-2, false});
    condition.restrictUpperBound(0, 0, Bounds{3, false});
    condition.restrictLowerBound(0, 1, Bounds{-4, true});
    condition.restrictUpperBound(0, 1, Bounds{4, true});
    condition.restrictLowerBound(0, 2, Bounds{-4, true});
    condition.restrictUpperBound(0, 2, Bounds{4, true});
    condition.restrictLowerBound(1, 1, Bounds{-1, false});
    condition.restrictUpperBound(1, 1, Bounds{2, false});
    condition.restrictLowerBound(1, 2, Bounds{-1, false});
    condition.restrictUpperBound(1, 2, Bounds{2, false});
    condition.restrictLowerBound(2, 2, Bounds{0, true});
    condition.restrictUpperBound(2, 2, Bounds{0, true});
    stream << condition;
    BOOST_CHECK_EQUAL("2 < T_{0, 0}  < 3 && 4 <= T_{0, 1}  <= 4 && 4 <= T_{0, 2}  <= 4 && 1 < T_{1, 1}  < 2 && 1 < T_{1, 2}  < 2 && -0 <= T_{2, 2}  <= 0", stream.str());
    auto inactiveClockVariables = ExternalTransitionMaker::inactiveClockVariables(renaming, condition);
    decltype(inactiveClockVariables) expected = {std::make_pair(1, 2)};
    BOOST_TEST(expected == inactiveClockVariables, boost::test_tools::per_element());
  }

  BOOST_AUTO_TEST_CASE(makeTest20230108) {
    std::stringstream stream;
    RenamingRelation renamingRelation;

    TimedCondition sourceCondition{ Zone::top(4)};
    sourceCondition.restrictLowerBound(0, 0, Bounds{-2, true});
    sourceCondition.restrictUpperBound(0, 0, Bounds{2, true});
    sourceCondition.restrictLowerBound(1, 1, Bounds{0, true});
    sourceCondition.restrictUpperBound(1, 1, Bounds{0, true});
    sourceCondition.restrictLowerBound(2, 2, Bounds{-3, true});
    sourceCondition.restrictUpperBound(2, 2, Bounds{3, true});
    stream << sourceCondition;
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 5 <= T_{0, 2}  <= 5 && -0 <= T_{1, 1}  <= 0 && 3 <= T_{1, 2}  <= 3 && 3 <= T_{2, 2}  <= 3",
                      stream.str());
    stream.str("");

    TimedCondition targetCondition {Zone::top(5)};
    targetCondition.restrictLowerBound(0, 0, Bounds{-2, true});
    targetCondition.restrictUpperBound(0, 0, Bounds{2, true});
    targetCondition.restrictLowerBound(1, 1, Bounds{0, true});
    targetCondition.restrictUpperBound(1, 1, Bounds{0, true});
    targetCondition.restrictLowerBound(2, 2, Bounds{-1, true});
    targetCondition.restrictUpperBound(2, 2, Bounds{1, true});
    targetCondition.restrictLowerBound(3, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << targetCondition;
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 3 <= T_{0, 2}  <= 3 && 3 <= T_{0, 3}  <= 3 && -0 <= T_{1, 1}  <= 0 && 1 <= T_{1, 2}  <= 1 && 1 <= T_{1, 3}  <= 1 && 1 <= T_{2, 2}  <= 1 && 1 <= T_{2, 3}  <= 1 && -0 <= T_{3, 3}  <= 0",
                      stream.str());
    stream.str("");

    ExternalTransitionMaker maker;
    maker.add(std::make_shared<TAState>(false), renamingRelation, sourceCondition, targetCondition);
    const auto transitions = maker.make();
    BOOST_CHECK_EQUAL(1, transitions.size());
    stream << transitions.front().resetVars;
    BOOST_CHECK_EQUAL("x0 := 3, x1 := 1, x2 := 1, x3 := 0", stream.str());
  }
BOOST_AUTO_TEST_SUITE_END()

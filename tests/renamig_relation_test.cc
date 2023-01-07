/**
 * @author Masaki Waga
 * @date 2022/12/09.
 */

#include <sstream>
#include <boost/test/unit_test.hpp>
#define protected public
#include "../include/timed_automaton.hh"
#include "../include/renaming_relation.hh"


BOOST_AUTO_TEST_SUITE(RecognizableLanguagesTest)
  using namespace learnta;
  BOOST_AUTO_TEST_CASE(toResetWithForceAssign1) {
    std::stringstream stream;
    // Make the source condition
    TimedCondition sourceCondition = TimedCondition{Zone::top(4)};
    sourceCondition.restrictLowerBound(0, 0, Bounds{0, false});
    sourceCondition.restrictUpperBound(0, 0, Bounds{1, false});
    sourceCondition.restrictLowerBound(1, 1, Bounds{0, true});
    sourceCondition.restrictUpperBound(1, 1, Bounds{0, true});
    sourceCondition.restrictLowerBound(2, 2, Bounds{0, true});
    sourceCondition.restrictUpperBound(2, 2, Bounds{0, true});
    stream << sourceCondition;
    BOOST_CHECK_EQUAL(stream.str(), "-0 < T_{0, 0}  < 1 && -0 < T_{0, 1}  < 1 && -0 < T_{0, 2}  < 1 && -0 <= T_{1, 1}  <= 0 && -0 <= T_{1, 2}  <= 0 && -0 <= T_{2, 2}  <= 0");
    stream.str("");

    // Make the target condition
    TimedCondition targetCondition = TimedCondition{Zone::top(5)};
    targetCondition.restrictLowerBound(0, 0, Bounds{0, true});
    targetCondition.restrictUpperBound(0, 0, Bounds{0, true});
    targetCondition.restrictLowerBound(1, 1, Bounds{0, true});
    targetCondition.restrictUpperBound(1, 1, Bounds{0, true});
    targetCondition.restrictLowerBound(2, 2, Bounds{0, true});
    targetCondition.restrictUpperBound(2, 2, Bounds{0, true});
    targetCondition.restrictLowerBound(3, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << targetCondition;
    BOOST_CHECK_EQUAL(stream.str(), "-0 <= T_{0, 0}  <= 0 && -0 <= T_{0, 1}  <= 0 && -0 <= T_{0, 2}  <= 0 && -0 <= T_{0, 3}  <= 0 && -0 <= T_{1, 1}  <= 0 && -0 <= T_{1, 2}  <= 0 && -0 <= T_{1, 3}  <= 0 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0");

    // Make the renaming relation
    RenamingRelation renaming;
    auto reset = renaming.toReset(sourceCondition, targetCondition);
    const std::vector<double> oldValuation = {0.5, 0.0, 0.0, 0.5};
    const std::vector<double> expectedValuation = {0.0, 0.0, 0.0, 0.0};
    std::vector<double> newValuation = oldValuation;

    BOOST_LOG_TRIVIAL(info) << "reset: " << reset;

    for (const auto &[resetVariable, targetVariable]: reset) {
      if (targetVariable.index() == 1) {
        newValuation.at(resetVariable) = oldValuation.at(std::get<ClockVariables>(targetVariable));
      } else {
        newValuation.at(resetVariable) = std::get<double>(targetVariable);
      }
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(newValuation.begin(), newValuation.end(),
                                  expectedValuation.begin(), expectedValuation.end());
  }

  BOOST_AUTO_TEST_CASE(toResetWithForceAssign2) {
    std::stringstream stream;
    // Make the source condition
    TimedCondition sourceCondition = TimedCondition{Zone::top(5)};
    sourceCondition.restrictLowerBound(0, 0, Bounds{0, false});
    sourceCondition.restrictUpperBound(0, 0, Bounds{1, false});
    sourceCondition.restrictLowerBound(1, 1, Bounds{0, true});
    sourceCondition.restrictUpperBound(1, 1, Bounds{0, true});
    sourceCondition.restrictLowerBound(2, 2, Bounds{0, true});
    sourceCondition.restrictUpperBound(2, 2, Bounds{0, true});
    sourceCondition.restrictLowerBound(3, 3, Bounds{0, true});
    sourceCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << sourceCondition;
    BOOST_CHECK_EQUAL(stream.str(), "-0 < T_{0, 0}  < 1 && -0 < T_{0, 1}  < 1 && -0 < T_{0, 2}  < 1 && -0 < T_{0, 3}  < 1 && -0 <= T_{1, 1}  <= 0 && -0 <= T_{1, 2}  <= 0 && -0 <= T_{1, 3}  <= 0 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0");
    stream.str("");

    // Make the target condition
    TimedCondition targetCondition = TimedCondition{Zone::top(5)};
    targetCondition.restrictLowerBound(0, 0, Bounds{0, true});
    targetCondition.restrictUpperBound(0, 0, Bounds{0, true});
    targetCondition.restrictLowerBound(1, 1, Bounds{0, true});
    targetCondition.restrictUpperBound(1, 1, Bounds{0, true});
    targetCondition.restrictLowerBound(2, 2, Bounds{0, true});
    targetCondition.restrictUpperBound(2, 2, Bounds{0, true});
    targetCondition.restrictLowerBound(3, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << targetCondition;
    BOOST_CHECK_EQUAL(stream.str(), "-0 <= T_{0, 0}  <= 0 && -0 <= T_{0, 1}  <= 0 && -0 <= T_{0, 2}  <= 0 && -0 <= T_{0, 3}  <= 0 && -0 <= T_{1, 1}  <= 0 && -0 <= T_{1, 2}  <= 0 && -0 <= T_{1, 3}  <= 0 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0");

    // Make the renaming relation
    RenamingRelation renaming;
    auto reset = renaming.toReset(sourceCondition, targetCondition);
    const std::vector<double> oldValuation = {0.5, 0.0, 0.0, 0.0};
    const std::vector<double> expectedValuation = {0.0, 0.0, 0.0, 0.0};
    std::vector<double> newValuation = oldValuation;

    BOOST_LOG_TRIVIAL(info) << "reset: " << reset;

    for (const auto &[resetVariable, targetVariable]: reset) {
      if (targetVariable.index() == 1) {
        newValuation.at(resetVariable) = oldValuation.at(std::get<ClockVariables>(targetVariable));
      } else {
        newValuation.at(resetVariable) = std::get<double>(targetVariable);
      }
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(newValuation.begin(), newValuation.end(),
                                  expectedValuation.begin(), expectedValuation.end());
  }

  BOOST_AUTO_TEST_CASE(toResetWithForceAssign3) {
    std::stringstream stream;
    // Make the source condition
    TimedCondition sourceCondition = TimedCondition{Zone::top(4)};
    sourceCondition.restrictLowerBound(0, 0, Bounds{0, true});
    sourceCondition.restrictUpperBound(0, 0, Bounds{0, true});
    sourceCondition.restrictLowerBound(1, 1, Bounds{0, false});
    sourceCondition.restrictUpperBound(1, 1, Bounds{1, false});
    sourceCondition.restrictLowerBound(2, 2, Bounds{0, true});
    sourceCondition.restrictUpperBound(2, 2, Bounds{0, true});
    stream << sourceCondition;
    BOOST_CHECK_EQUAL(stream.str(), "-0 <= T_{0, 0}  <= 0 && -0 < T_{0, 1}  < 1 && -0 < T_{0, 2}  < 1 && -0 < T_{1, 1}  < 1 && -0 < T_{1, 2}  < 1 && -0 <= T_{2, 2}  <= 0");
    stream.str("");

    // Make the target condition
    TimedCondition targetCondition = TimedCondition{Zone::top(5)};
    targetCondition.restrictLowerBound(0, 0, Bounds{0, true});
    targetCondition.restrictUpperBound(0, 0, Bounds{0, true});
    targetCondition.restrictLowerBound(1, 1, Bounds{0, true});
    targetCondition.restrictUpperBound(1, 1, Bounds{0, true});
    targetCondition.restrictLowerBound(2, 2, Bounds{0, true});
    targetCondition.restrictUpperBound(2, 2, Bounds{0, true});
    targetCondition.restrictLowerBound(3, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << targetCondition;
    BOOST_CHECK_EQUAL(stream.str(), "-0 <= T_{0, 0}  <= 0 && -0 <= T_{0, 1}  <= 0 && -0 <= T_{0, 2}  <= 0 && -0 <= T_{0, 3}  <= 0 && -0 <= T_{1, 1}  <= 0 && -0 <= T_{1, 2}  <= 0 && -0 <= T_{1, 3}  <= 0 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0");

    // Make the renaming relation
    RenamingRelation renaming;
    auto reset = renaming.toReset(sourceCondition, targetCondition);
    const std::vector<double> oldValuation = {0.5, 0.5, 0.0, 0.5};
    const std::vector<double> expectedValuation = {0.0, 0.0, 0.0, 0.0};
    std::vector<double> newValuation = oldValuation;

    BOOST_LOG_TRIVIAL(info) << "reset: " << reset;

    for (const auto &[resetVariable, targetVariable]: reset) {
      if (targetVariable.index() == 1) {
        newValuation.at(resetVariable) = oldValuation.at(std::get<ClockVariables>(targetVariable));
      } else {
        newValuation.at(resetVariable) = std::get<double>(targetVariable);
      }
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(newValuation.begin(), newValuation.end(),
                                  expectedValuation.begin(), expectedValuation.end());
  }

  // codomain: (abb, 1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0) renaming: {t0 == t'1}
  BOOST_AUTO_TEST_CASE(fullAndEmpty) {
    std::stringstream stream;
    auto condition = TimedCondition{Zone::top(5)};
    condition.restrictLowerBound(0, 0, Bounds{-1, true});
    condition.restrictUpperBound(0, 0, Bounds{1, true});
    condition.restrictLowerBound(0, 1, Bounds{-3, false});
    condition.restrictUpperBound(0, 1, Bounds{4, false});
    condition.restrictLowerBound(0, 2, Bounds{-3, false});
    condition.restrictUpperBound(0, 2, Bounds{4, false});
    condition.restrictLowerBound(0, 3, Bounds{-3, false});
    condition.restrictUpperBound(0, 3, Bounds{4, false});
    condition.restrictLowerBound(1, 1, Bounds{-2, false});
    condition.restrictUpperBound(1, 1, Bounds{3, false});
    condition.restrictLowerBound(1, 2, Bounds{-2, false});
    condition.restrictUpperBound(1, 2, Bounds{3, false});
    condition.restrictLowerBound(1, 3, Bounds{-2, false});
    condition.restrictUpperBound(1, 3, Bounds{3, false});
    condition.restrictLowerBound(2, 2, Bounds{0, true});
    condition.restrictUpperBound(2, 2, Bounds{0, true});
    condition.restrictLowerBound(2, 3, Bounds{0, true});
    condition.restrictUpperBound(2, 3, Bounds{0, true});
    condition.restrictLowerBound(3, 3, Bounds{0, true});
    condition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << condition;
    BOOST_CHECK_EQUAL(stream.str(),
                      "1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0");
    stream.str("");

    RenamingRelation renaming;
    renaming.emplace_back(static_cast<ClockVariables>(0), static_cast<ClockVariables>(1));
    stream << renaming;
    BOOST_CHECK_EQUAL(stream.str(), "{t0 == t'1}");
    stream.str("");
    BOOST_TEST(!renaming.empty());
    BOOST_TEST(!renaming.full(condition));
  }

  BOOST_AUTO_TEST_CASE(rightVariables) {
    std::stringstream stream;
    RenamingRelation renaming;
    renaming.emplace_back(static_cast<ClockVariables>(0), static_cast<ClockVariables>(1));
    stream << renaming;
    BOOST_CHECK_EQUAL(stream.str(), "{t0 == t'1}");
    stream.str("");
    std::vector<ClockVariables> expectedRightVariables = {1};
    BOOST_TEST(expectedRightVariables == renaming.rightVariables(), boost::test_tools::per_element());
  }

  BOOST_AUTO_TEST_CASE(addImplicitConstraintsTest) {
    std::stringstream stream;

    // Make the source condition
    TimedCondition sourceCondition = TimedCondition{Zone::top(5)};
    sourceCondition.restrictLowerBound(0, 0, Bounds{-1, true});
    sourceCondition.restrictUpperBound(0, 0, Bounds{1, true});
    sourceCondition.restrictLowerBound(0, 1, Bounds{-3, false});
    sourceCondition.restrictUpperBound(0, 1, Bounds{4, false});
    sourceCondition.restrictLowerBound(0, 2, Bounds{-3, false});
    sourceCondition.restrictUpperBound(0, 2, Bounds{4, false});
    sourceCondition.restrictLowerBound(0, 3, Bounds{-3, true});
    sourceCondition.restrictUpperBound(0, 3, Bounds{4, true});
    sourceCondition.restrictLowerBound(1, 1, Bounds{-2, false});
    sourceCondition.restrictUpperBound(1, 1, Bounds{3, false});
    sourceCondition.restrictLowerBound(1, 2, Bounds{-2, false});
    sourceCondition.restrictUpperBound(1, 2, Bounds{3, false});
    sourceCondition.restrictLowerBound(1, 3, Bounds{-2, false});
    sourceCondition.restrictUpperBound(1, 3, Bounds{3, false});
    sourceCondition.restrictLowerBound(2, 2, Bounds{0, false});
    sourceCondition.restrictUpperBound(2, 2, Bounds{1, false});
    sourceCondition.restrictLowerBound(2, 3, Bounds{0, false});
    sourceCondition.restrictUpperBound(2, 3, Bounds{1, false});
    sourceCondition.restrictLowerBound(3, 3, Bounds{0, true});
    sourceCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << sourceCondition;
    BOOST_CHECK_EQUAL(stream.str(), "1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && -0 < T_{2, 2}  < 1 && -0 < T_{2, 3}  < 1 && -0 <= T_{3, 3}  <= 0");
    stream.str("");

    // Make the target condition
    TimedCondition targetCondition = TimedCondition{Zone::top(5)};
    targetCondition.restrictLowerBound(0, 0, Bounds{-1, true});
    targetCondition.restrictUpperBound(0, 0, Bounds{1, true});
    targetCondition.restrictLowerBound(0, 1, Bounds{-3, false});
    targetCondition.restrictUpperBound(0, 1, Bounds{4, false});
    targetCondition.restrictLowerBound(0, 2, Bounds{-3, false});
    targetCondition.restrictUpperBound(0, 2, Bounds{4, false});
    targetCondition.restrictLowerBound(0, 3, Bounds{-3, false});
    targetCondition.restrictUpperBound(0, 3, Bounds{4, false});
    targetCondition.restrictLowerBound(1, 1, Bounds{-2, false});
    targetCondition.restrictUpperBound(1, 1, Bounds{3, false});
    targetCondition.restrictLowerBound(1, 2, Bounds{-2, false});
    targetCondition.restrictUpperBound(1, 2, Bounds{3, false});
    targetCondition.restrictLowerBound(1, 3, Bounds{-2, false});
    targetCondition.restrictUpperBound(1, 3, Bounds{3, false});
    targetCondition.restrictLowerBound(2, 2, Bounds{0, true});
    targetCondition.restrictUpperBound(2, 2, Bounds{0, true});
    targetCondition.restrictLowerBound(2, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(2, 3, Bounds{0, true});
    targetCondition.restrictLowerBound(3, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << targetCondition;
    BOOST_CHECK_EQUAL(stream.str(), "1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0");
    stream.str("");

    // Make the renaming relation without implicit renaming equation
    RenamingRelation renaming;
    renaming.emplace_back(static_cast<ClockVariables>(0), static_cast<ClockVariables>(0));
    stream << renaming;
    BOOST_CHECK_EQUAL(stream.str(), "{t0 == t'0}");
    stream.str("");

    renaming.addImplicitConstraints(sourceCondition, targetCondition);
    stream << renaming;
    BOOST_CHECK_EQUAL(stream.str(), "{t0 == t'0 && t1 == t'1 && t3 == t'2 && t3 == t'3}");
  }

  BOOST_AUTO_TEST_CASE(makeRenamingTest) {
    std::stringstream stream;

    // Make the source condition
    // We this is the source condition after discrete successor
    TimedCondition sourceCondition = TimedCondition{Zone::top(2)};
    sourceCondition.restrictLowerBound(0, 0, Bounds{-2, false});
    sourceCondition.restrictUpperBound(0, 0, Bounds{3, false});
    stream << sourceCondition;
    BOOST_CHECK_EQUAL(stream.str(), "2 < T_{0, 0}  < 3");
    stream.str("");

    // Make the target condition
    TimedCondition targetCondition = TimedCondition{Zone::top(5)};
    targetCondition.restrictLowerBound(0, 0, Bounds{-1, true});
    targetCondition.restrictUpperBound(0, 0, Bounds{1, true});
    targetCondition.restrictLowerBound(0, 1, Bounds{-3, false});
    targetCondition.restrictUpperBound(0, 1, Bounds{4, false});
    targetCondition.restrictLowerBound(0, 2, Bounds{-3, false});
    targetCondition.restrictUpperBound(0, 2, Bounds{4, false});
    targetCondition.restrictLowerBound(0, 3, Bounds{-3, false});
    targetCondition.restrictUpperBound(0, 3, Bounds{4, false});
    targetCondition.restrictLowerBound(1, 1, Bounds{-2, false});
    targetCondition.restrictUpperBound(1, 1, Bounds{3, false});
    targetCondition.restrictLowerBound(1, 2, Bounds{-2, false});
    targetCondition.restrictUpperBound(1, 2, Bounds{3, false});
    targetCondition.restrictLowerBound(1, 3, Bounds{-2, false});
    targetCondition.restrictUpperBound(1, 3, Bounds{3, false});
    targetCondition.restrictLowerBound(2, 2, Bounds{0, true});
    targetCondition.restrictUpperBound(2, 2, Bounds{0, true});
    targetCondition.restrictLowerBound(2, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(2, 3, Bounds{0, true});
    targetCondition.restrictLowerBound(3, 3, Bounds{0, true});
    targetCondition.restrictUpperBound(3, 3, Bounds{0, true});
    stream << targetCondition;
    BOOST_CHECK_EQUAL(stream.str(), "1 <= T_{0, 0}  <= 1 && 3 < T_{0, 1}  < 4 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 2 < T_{1, 1}  < 3 && 2 < T_{1, 2}  < 3 && 2 < T_{1, 3}  < 3 && -0 <= T_{2, 2}  <= 0 && -0 <= T_{2, 3}  <= 0 && -0 <= T_{3, 3}  <= 0");
    stream.str("");

    // Make the renaming relation without implicit renaming equation
    RenamingRelation renaming;
    renaming.emplace_back(static_cast<ClockVariables>(0), static_cast<ClockVariables>(1));
    renaming.emplace_back(static_cast<ClockVariables>(1), static_cast<ClockVariables>(2));
    renaming.emplace_back(static_cast<ClockVariables>(1), static_cast<ClockVariables>(3));
    stream << renaming;
    BOOST_CHECK_EQUAL(stream.str(), "{t0 == t'1 && t1 == t'2 && t1 == t'3}");
    stream.str("");

    auto juxtaposedCondition = sourceCondition.extendN() ^ targetCondition;
    juxtaposedCondition.addRenaming(renaming);
    const auto newRenamingRelation = RenamingRelation{juxtaposedCondition.makeRenaming()};

    stream << newRenamingRelation;
    BOOST_CHECK_EQUAL(stream.str(), "{t0 == t'1 && t1 == t'2 && t1 == t'3}");
  }
BOOST_AUTO_TEST_SUITE_END()
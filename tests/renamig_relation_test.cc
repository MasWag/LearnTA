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

BOOST_AUTO_TEST_SUITE_END()
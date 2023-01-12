/**
 * @author Masaki Waga
 * @date 2023/01/12.
 */
#include <boost/test/unit_test.hpp>

#include "imprecise_clock_handler.hh"


BOOST_AUTO_TEST_SUITE(ImpreciseClockHandlerTest)
  using namespace learnta;

  BOOST_AUTO_TEST_CASE(embedIfImpreciseTest) {
    TATransition::Resets resets;
    resets.emplace_back(static_cast<ClockVariables>(0), static_cast<ClockVariables>(8));
    resets.emplace_back(static_cast<ClockVariables>(1), static_cast<ClockVariables>(7));
    std::unordered_set<ClockVariables> preciseClocks = {1};
    std::vector<double> valuation = {2.5, 2.0, 1.5};
    TATransition::Resets expectedResets;
    expectedResets.emplace_back(static_cast<ClockVariables>(1), static_cast<ClockVariables>(7));
    expectedResets.emplace_back(static_cast<ClockVariables>(0), 2.5);
    expectedResets.emplace_back(static_cast<ClockVariables>(2), 1.5);
    BOOST_TEST(expectedResets == ImpreciseClockHandler::embedIfImprecise(resets, preciseClocks, valuation),
               boost::test_tools::per_element());
  }
BOOST_AUTO_TEST_SUITE_END()
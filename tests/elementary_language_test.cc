/**
 * @author Masaki Waga
 * @date 2022/03/03.
 */
#define protected public

#include "../include/elementary_language.hh"

using namespace learnta;

#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(ElementaryLanguageTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(sampleOpen) {
    TimedCondition timedCondition;
    // left is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    timedCondition.zone = Zone::top(3);
    timedCondition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    timedCondition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    timedCondition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    timedCondition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    timedCondition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    timedCondition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0
    ElementaryLanguage elementary = {"a", timedCondition};

    TimedWord sample = elementary.sample();
    BOOST_CHECK_EQUAL("a", sample.getWord());
    BOOST_REQUIRE_EQUAL(2, sample.getDurations().size());
    BOOST_CHECK_EQUAL(0.5, sample.getDurations()[0]);
    BOOST_CHECK_EQUAL(0.5, sample.getDurations()[1]);
  }

  BOOST_AUTO_TEST_CASE(sampleClosed) {
    TimedCondition timedCondition;
    // left is \tau_0 = 1 && \tau_0 + \tau_1 \in (1, 2) && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 <= 1 && x2 - x1 <= -1 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 < 1 && x0 - x2 < 0
    timedCondition.zone = Zone::top(3);
    timedCondition.zone.tighten(0, 1, {1, true}); // x1 - x2 <= 1
    timedCondition.zone.tighten(1, 0, {-1, true}); // x2 - x1 <= -1
    timedCondition.zone.tighten(0, -1, {2, false}); // x1 - x0 < 2
    timedCondition.zone.tighten(-1, 0, {-1, false}); // x0 - x1 < -1
    timedCondition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    timedCondition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0
    ElementaryLanguage elementary = {"a", timedCondition};

    TimedWord sample = elementary.sample();
    BOOST_CHECK_EQUAL("a", sample.getWord());
    BOOST_REQUIRE_EQUAL(2, sample.getDurations().size());
    BOOST_CHECK_EQUAL(1, sample.getDurations()[0]);
    BOOST_CHECK_EQUAL(0.5, sample.getDurations()[1]);
  }

BOOST_AUTO_TEST_SUITE_END()

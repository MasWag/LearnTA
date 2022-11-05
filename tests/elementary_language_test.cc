/**
 * @author Masaki Waga
 * @date 2022/03/03.
 */

#include <sstream>

#define protected public

#include "../include/elementary_language.hh"

using namespace learnta;

#include <boost/test/unit_test.hpp>

#include "simple_observation_table_keys_fixture.hh"

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

  BOOST_FIXTURE_TEST_CASE(concatenation, SimpleObservationTableKeysFixture) {
    std::stringstream ss;
    std::string result;
    // p1s3 should be (a, -0 < T_{0, 0}  < 1 && -0 < T_{0, 1}  < 1 && -0 <= T_{1, 1}  <= 0)
    auto p1s3 = p1 + s3;
    // std::cout << p1s3.getTimedCondition() << std::endl;
    ss << p1s3.getTimedCondition();
    BOOST_CHECK_EQUAL("-0 < T_{0, 0}  < 1 && -0 < T_{0, 1}  < 1 && -0 <= T_{1, 1}  <= 0", ss.str());
    ss.str("");

    // p2s3 should be (a, -0 < T_{0, 0}  < 2 && -0 < T_{0, 1}  < 2 && -0 < T_{1, 1}  < 1)
    auto p2s3 = p2 + s3;
    // std::cout << p2s3.getTimedCondition() << std::endl;
    ss << p2s3.getTimedCondition();
    BOOST_CHECK_EQUAL("-0 < T_{0, 0}  < 2 && -0 < T_{0, 1}  < 2 && -0 <= T_{1, 1}  <= 0", ss.str());
    ss.str("");

    // p5s1 should be T_{0, 0} = 1 && 1 < T_{0, 1}  < 2 && 0 < T_{1, 1}  < 1
    auto p5s1 = p5 + s1;
    // std::cout << p5s1.getTimedCondition() << std::endl;
    ss << p5s1.getTimedCondition();
    BOOST_CHECK_EQUAL("1 <= T_{0, 0}  <= 1 && 1 < T_{0, 1}  < 2 && -0 < T_{1, 1}  < 1", ss.str());
    ss.str("");

    // p5s3 should be T_{0, 0} = 1 && 1 < T_{0, 1}  < 3 && 1 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 2 && -0 < T_{1, 2}  < 2 && -0 <= T_{2, 2}  <= 0
    auto p5s3 = p5 + s3;
    // std::cout << p5s3.getTimedCondition() << std::endl;
    ss << p5s3.getTimedCondition();
    BOOST_CHECK_EQUAL("1 <= T_{0, 0}  <= 1 && 1 < T_{0, 1}  < 3 && 1 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 2 && -0 < T_{1, 2}  < 2 && -0 <= T_{2, 2}  <= 0", ss.str());
    ss.str("");
  }

  BOOST_AUTO_TEST_CASE(contains) {
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

    // 1.0 - 0.7 != 0.3 in floating number!!
    BOOST_TEST(elementary.contains(TimedWord{"a", {0.7, 1.0 - 0.7}}));
    BOOST_TEST(elementary.contains(TimedWord{"a", {0.5, 0.5}}));
    BOOST_TEST(!elementary.contains(TimedWord{"a", {0.5, 0.3}}));
    BOOST_TEST(!elementary.contains(TimedWord{"a", {0.0, 1.0}}));
    BOOST_TEST(!elementary.contains(TimedWord{"a", {1.0, 0.0}}));
  }

  BOOST_AUTO_TEST_CASE(containsZero) {
    const auto word = TimedWord{"", {1.75}};
    auto zero = ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"", {0}});
    BOOST_CHECK(!zero.contains(word));
  }
BOOST_AUTO_TEST_SUITE_END()

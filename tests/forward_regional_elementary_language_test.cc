/**
 * @author Masaki Waga
 * @date 2022/03/03.
 */
#include "../include/bounds.hh"
#include <boost/test/unit_test.hpp>

#define protected public
#define private public

#include "../include/forward_regional_elementary_language.hh"
#include "simple_observation_table_keys_fixture.hh"

BOOST_AUTO_TEST_SUITE(ForwardRegionalElementaryLanguageTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(continuousSuccessor) {
    TimedCondition timedCondition;
    // timedCondition is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    timedCondition.zone = Zone::top(3);
    timedCondition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    timedCondition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    timedCondition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    timedCondition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    timedCondition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    timedCondition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0
    FractionalOrder order;
    order.order.push_back({1});
    order.size = 2;
    ForwardRegionalElementaryLanguage elementary = {{"a", timedCondition}, order};

    auto continuousSuccessor = elementary.successor();
    BOOST_CHECK_EQUAL("a", continuousSuccessor.word);
    // continuousSuccessor.timedCondition is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (1, 2) && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousSuccessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousSuccessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), continuousSuccessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), continuousSuccessor.timedCondition.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousSuccessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousSuccessor.timedCondition.zone.value(0, 2));
    // Check the fractional order
    BOOST_REQUIRE_EQUAL(2, continuousSuccessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(3, continuousSuccessor.fractionalOrder.order.size());
    auto it = continuousSuccessor.fractionalOrder.order.begin();
    BOOST_CHECK(it->empty());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->front());

    continuousSuccessor = continuousSuccessor.successor();
    BOOST_CHECK_EQUAL("a", continuousSuccessor.word);
    // continuousSuccessor.timedCondition is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (1, 2) && \tau_1 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 <= 1 && x0 - x2 <= -1
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousSuccessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousSuccessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), continuousSuccessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), continuousSuccessor.timedCondition.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), continuousSuccessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), continuousSuccessor.timedCondition.zone.value(0, 2));
    // Check the fractional order
    BOOST_REQUIRE_EQUAL(2, continuousSuccessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(2, continuousSuccessor.fractionalOrder.order.size());
    it = continuousSuccessor.fractionalOrder.order.begin();
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
  }

  BOOST_AUTO_TEST_CASE(discreteSuccessor) {
    TimedCondition timedCondition;
    // timedCondition is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    timedCondition.zone = Zone::top(3);
    timedCondition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    timedCondition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    timedCondition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    timedCondition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    timedCondition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    timedCondition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0
    FractionalOrder order;
    BOOST_REQUIRE_EQUAL(1, order.order.front().size());
    order.order.push_back(std::list<ClockVariables>{1});
    order.size = 2;
    ForwardRegionalElementaryLanguage elementary = {{"a", timedCondition}, order};

    auto discreteSuccessor = elementary.successor('b');
    BOOST_CHECK_EQUAL("ab", discreteSuccessor.word);
    // discreteSuccessor.timedCondition is
    //   - \tau_0 \in (0,1) &&
    //   - \tau_1 \in (0,1) &&
    //   - \tau_2 = 0
    //   - \tau_0 + \tau_1 = 1 &&
    //   - \tau_1 + \tau_2 \in (0, 1) &&
    //   - \tau_0 + \tau_1 + \tau_2 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1 + \tau_2, x2 == \tau_1 + \tau_2, and x3 == \tau_2.
    // Therefore, we have
    //   - x1 - x2 < 1 && x2 - x1 < 0 &&
    //   - x2 - x3 < 1 && x3 - x2 < 0 &&
    //   - x3 - x0 <= 0 && x0 - x3 <= 0 &&
    //   - x1 - x3 <= 1 && x3 - x1 <= -1 &&
    //   - x2 - x0 < 1 && x0 - x2 < 0
    //   - x1 - x0 <= 1 && x0 - x1 <= -1 &&
    BOOST_CHECK_EQUAL((Bounds{1, false}), discreteSuccessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discreteSuccessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discreteSuccessor.timedCondition.zone.value(2, 3));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discreteSuccessor.timedCondition.zone.value(3, 2));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(3, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(0, 3));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discreteSuccessor.timedCondition.zone.value(1, 3));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discreteSuccessor.timedCondition.zone.value(3, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discreteSuccessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discreteSuccessor.timedCondition.zone.value(0, 2));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discreteSuccessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discreteSuccessor.timedCondition.zone.value(0, 1));

    // Check the fractional order
    BOOST_REQUIRE_EQUAL(3, discreteSuccessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(2, discreteSuccessor.fractionalOrder.order.size());
    auto it = discreteSuccessor.fractionalOrder.order.begin();
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    BOOST_CHECK_EQUAL(2, it->back());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->front());

    discreteSuccessor = discreteSuccessor.successor('a');
    BOOST_CHECK_EQUAL("aba", discreteSuccessor.word);
    // discreteSuccessor.timedCondition is
    //   - \tau_0 \in (0,1) &&
    //   - \tau_1 \in (0,1) &&
    //   - \tau_2 = 0
    //   - \tau_3 = 0
    //   - \tau_0 + \tau_1 = 1 &&
    //   - \tau_1 + \tau_2 \in (0, 1) &&
    //   - \tau_2 + \tau_3 = 0 &&
    //   - \tau_0 + \tau_1 + \tau_2 = 1 &&
    //   - \tau_1 + \tau_2 + \tau_3 \in (0, 1) &&
    //   - \tau_0 + \tau_1 + \tau_2 + \tau_3 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1 + \tau_2 + \tau_3, x2 == \tau_1 + \tau_2 + \tau_3, x3 == \tau_2 + \tau_3, and x4 == \tau_3.
    // Therefore, we have
    //   - x1 - x2 < 1 && x2 - x1 < 0 &&
    //   - x2 - x3 < 1 && x3 - x2 < 0 &&
    //   - x3 - x4 <= 0 && x4 - x3 <= 0 &&
    //   - x4 - x0 <= 0 && x0 - x4 <= 0 &&
    //   - x1 - x3 <= 1 && x3 - x1 <= -1 &&
    //   - x2 - x4 < 1 && x4 - x2 < 0
    //   - x3 - x0 <= 0 && x0 - x3 < =0
    //   - x1 - x4 <= 1 && x4 - x1 <= -1 &&
    //   - x2 - x0 < 0 && x0 - x2 < -1
    //   - x1 - x0 <= 1 && x0 - x1 <= -1 &&
    BOOST_CHECK_EQUAL((Bounds{1, false}), discreteSuccessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discreteSuccessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discreteSuccessor.timedCondition.zone.value(2, 3));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discreteSuccessor.timedCondition.zone.value(3, 2));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(3, 4));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(4, 3));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(4, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(0, 4));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discreteSuccessor.timedCondition.zone.value(1, 3));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discreteSuccessor.timedCondition.zone.value(3, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discreteSuccessor.timedCondition.zone.value(2, 4));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discreteSuccessor.timedCondition.zone.value(4, 2));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(3, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discreteSuccessor.timedCondition.zone.value(0, 3));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discreteSuccessor.timedCondition.zone.value(1, 4));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discreteSuccessor.timedCondition.zone.value(4, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discreteSuccessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discreteSuccessor.timedCondition.zone.value(0, 2));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discreteSuccessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discreteSuccessor.timedCondition.zone.value(0, 1));
    // Check the fractional order
    BOOST_CHECK_EQUAL(4, discreteSuccessor.fractionalOrder.size);
    BOOST_CHECK_EQUAL(2, discreteSuccessor.fractionalOrder.order.size());
    it = discreteSuccessor.fractionalOrder.order.begin();
    BOOST_CHECK_EQUAL(3, it->size());
    auto ij = it->begin();
    BOOST_CHECK_EQUAL(0, *ij++);
    BOOST_CHECK_EQUAL(2, *ij++);
    BOOST_CHECK_EQUAL(3, *ij++);
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
  }

  BOOST_AUTO_TEST_CASE(fromWord) {
    const auto emptyWord = ForwardRegionalElementaryLanguage::fromTimedWord({"", {0}});

    BOOST_CHECK_EQUAL("", emptyWord.getWord());
    BOOST_CHECK_EQUAL(1, emptyWord.getTimedCondition().size());
    BOOST_CHECK_EQUAL((Bounds{0, true}), emptyWord.getTimedCondition().getUpperBound(0, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), emptyWord.getTimedCondition().getLowerBound(0, 0));
    BOOST_CHECK_EQUAL(1, emptyWord.fractionalOrder.order.size());
    BOOST_CHECK_EQUAL(1, emptyWord.fractionalOrder.order.front().size());
    BOOST_CHECK_EQUAL(0, emptyWord.fractionalOrder.order.front().front());
  }

  BOOST_AUTO_TEST_CASE(fromWord_2022_03_27) {
    const auto word = ForwardRegionalElementaryLanguage::fromTimedWord({"aaa", {2,1,0.5,0}});
    BOOST_CHECK_EQUAL(4, word.fractionalOrder.size);
    BOOST_CHECK_EQUAL(2, word.fractionalOrder.order.size());
    BOOST_CHECK_EQUAL(1, word.fractionalOrder.order.front().size());
    BOOST_CHECK_EQUAL(3, word.fractionalOrder.order.front().front());
    BOOST_CHECK_EQUAL(3, word.fractionalOrder.order.back().size());
    BOOST_CHECK_EQUAL(0, word.fractionalOrder.order.back().front());
    BOOST_CHECK_EQUAL(2, word.fractionalOrder.order.back().back());
  }

  BOOST_FIXTURE_TEST_CASE(p7HasEqualityN, SimpleObservationTableKeysFixture) {
    std::cout << p7 << std::endl;
    BOOST_CHECK(!p7.hasEqualityN());
  }

  BOOST_AUTO_TEST_SUITE_END()

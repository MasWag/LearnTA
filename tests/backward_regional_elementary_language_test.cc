/**
 * @author Masaki Waga
 * @date 2022/03/07.
 */
#include "../include/bounds.hh"
#include <boost/test/unit_test.hpp>

#define protected public
#define private public

#include "../include/backward_regional_elementary_language.hh"


BOOST_AUTO_TEST_SUITE(BackwardRegionalElementaryLanguageTest)

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
    BackwardRegionalElementaryLanguage elementary = {{"a", timedCondition}, order};

    auto continuousPredecessor = elementary.predecessor();
    BOOST_CHECK_EQUAL("a", continuousPredecessor.word);
    // continuousPredecessor.timedCondition is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (0, 1) && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 1 && x0 - x1 < 0 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousPredecessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousPredecessor.timedCondition.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousPredecessor.timedCondition.zone.value(0, 2));
    // Check the fractional order
    BOOST_REQUIRE_EQUAL(2, continuousPredecessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(3, continuousPredecessor.fractionalOrder.order.size());
    auto it = continuousPredecessor.fractionalOrder.order.begin();
    BOOST_CHECK(it->empty());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());

    continuousPredecessor = continuousPredecessor.predecessor();
    BOOST_CHECK_EQUAL("a", continuousPredecessor.word); // here
    // continuousSuccessor.timedCondition is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (0, 1) && \tau_1 = 0
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 <= 1 && x0 - x2 <= -1
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousPredecessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-0, false}), continuousPredecessor.timedCondition.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{0, true}), continuousPredecessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), continuousPredecessor.timedCondition.zone.value(0, 2));
    // Check the fractional order
    BOOST_REQUIRE_EQUAL(2, continuousPredecessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(2, continuousPredecessor.fractionalOrder.order.size());
    it = continuousPredecessor.fractionalOrder.order.begin();
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
    BackwardRegionalElementaryLanguage elementary = {{"a", timedCondition}, order};

    auto discretePredecessor = elementary.predecessor('b');
    BOOST_CHECK_EQUAL("ba", discretePredecessor.word);
    // discreteSuccessor.timedCondition is
    //   - \tau_0 = 0
    //   - \tau_1 \in (0,1) &&
    //   - \tau_2 \in (0,1) &&
    //   - \tau_0 + \tau_1 \in (0, 1) &&
    //   - \tau_1 + \tau_2 = 1 &&
    //   - \tau_0 + \tau_1 + \tau_2 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1 + \tau_2, x2 == \tau_1 + \tau_2, and x3 == \tau_2.
    // Therefore, we have
    //   - x1 - x2 <= 0 && x2 - x1 <= 0 &&
    //   - x2 - x3 < 1 && x3 - x2 < 0 &&
    //   - x3 - x0 < 1 && x0 - x3 < 0 &&
    //   - x1 - x3 < 1 && x3 - x1 < 0 &&
    //   - x2 - x0 <= 1 && x0 - x2 <= -1 &&
    //   - x1 - x0 <= 1 && x0 - x1 <= -1
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discretePredecessor.timedCondition.zone.value(2, 3));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discretePredecessor.timedCondition.zone.value(3, 2));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discretePredecessor.timedCondition.zone.value(3, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discretePredecessor.timedCondition.zone.value(0, 3));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discretePredecessor.timedCondition.zone.value(1, 3));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discretePredecessor.timedCondition.zone.value(3, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discretePredecessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discretePredecessor.timedCondition.zone.value(0, 2));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discretePredecessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discretePredecessor.timedCondition.zone.value(0, 1));

    // Check the fractional order
    BOOST_REQUIRE_EQUAL(3, discretePredecessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(2, discretePredecessor.fractionalOrder.order.size());
    auto it = discretePredecessor.fractionalOrder.order.begin();
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    BOOST_CHECK_EQUAL(1, it->back());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(2, it->front());

    discretePredecessor = discretePredecessor.predecessor('a');
    BOOST_CHECK_EQUAL("aba", discretePredecessor.word);
    // discreteSuccessor.timedCondition is
    //   - \tau_0 = 0
    //   - \tau_1 = 0
    //   - \tau_2 \in (0,1) &&
    //   - \tau_3 \in (0,1) &&
    //   - \tau_0 + \tau_1 \in 0 &&
    //   - \tau_1 + \tau_2 \in (0, 1) &&
    //   - \tau_2 + \tau_3 = 1 &&
    //   - \tau_0 + \tau_1 + \tau_2 \in (0, 1)
    //   - \tau_1 + \tau_2 + \tau_3 = 1
    //   - \tau_0 + \tau_1 + \tau_2 + \tau_3 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1 + \tau_2 + \tau_3, x2 == \tau_1 + \tau_2 + \tau_3, x3 == \tau_2 + \tau_3, and x4 == \tau_3.
    // Therefore, we have
    //   - x1 - x2 <= 0 && x2 - x1 <= 0 &&
    //   - x2 - x3 <= 0 && x3 - x2 <= 0 &&
    //   - x3 - x4 < 1 && x4 - x3 < 0 &&
    //   - x4 - x0 < 1 && x0 - x4 < 0 &&
    //   - x1 - x3 <= 0 && x3 - x1 <= 0 &&
    //   - x2 - x4 < 1 && x4 - x2 < 0 &&
    //   - x3 - x0 <= 1 && x0 - x3 <= -1 &&
    //   - x1 - x4 < 1 && x4 - x1 < 0 &&
    //   - x2 - x0 <= 1 && x0 - x2 <= -1 &&
    //   - x1 - x0 <= 1 && x0 - x1 <= -1
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(2, 3));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(3, 2));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discretePredecessor.timedCondition.zone.value(3, 4));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discretePredecessor.timedCondition.zone.value(4, 3));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discretePredecessor.timedCondition.zone.value(4, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discretePredecessor.timedCondition.zone.value(0, 4));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(1, 3));
    BOOST_CHECK_EQUAL((Bounds{0, true}), discretePredecessor.timedCondition.zone.value(3, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discretePredecessor.timedCondition.zone.value(2, 4));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discretePredecessor.timedCondition.zone.value(4, 2));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discretePredecessor.timedCondition.zone.value(3, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discretePredecessor.timedCondition.zone.value(0, 3));
    BOOST_CHECK_EQUAL((Bounds{1, false}), discretePredecessor.timedCondition.zone.value(1, 4));
    BOOST_CHECK_EQUAL((Bounds{0, false}), discretePredecessor.timedCondition.zone.value(4, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discretePredecessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discretePredecessor.timedCondition.zone.value(0, 2));
    BOOST_CHECK_EQUAL((Bounds{1, true}), discretePredecessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), discretePredecessor.timedCondition.zone.value(0, 1));
    // Check the fractional order
    BOOST_CHECK_EQUAL(4, discretePredecessor.fractionalOrder.size);
    BOOST_CHECK_EQUAL(2, discretePredecessor.fractionalOrder.order.size());
    it = discretePredecessor.fractionalOrder.order.begin();
    BOOST_CHECK_EQUAL(3, it->size());
    auto ij = it->begin();
    BOOST_CHECK_EQUAL(0, *ij++);
    BOOST_CHECK_EQUAL(1, *ij++);
    BOOST_CHECK_EQUAL(2, *ij++);
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
  }

BOOST_AUTO_TEST_SUITE_END()

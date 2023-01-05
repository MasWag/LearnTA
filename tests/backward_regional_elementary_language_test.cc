/**
 * @author Masaki Waga
 * @date 2022/03/07.
 */
#include "../include/bounds.hh"
#include <boost/test/unit_test.hpp>

#define protected public
#define private public

#include "../include/backward_regional_elementary_language.hh"

#include "simple_observation_table_keys_fixture.hh"

BOOST_AUTO_TEST_SUITE(BackwardRegionalElementaryLanguageTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(continuousPredecessor) {
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
    order.order = {{1}, {0}};
    order.size = 2;
    BackwardRegionalElementaryLanguage elementary = {{"a", timedCondition}, order};

    auto continuousPredecessor = elementary.predecessor();
    BOOST_CHECK_EQUAL("a", continuousPredecessor.word);
    // continuousPredecessor.timedCondition is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (1, 2) && \tau_1 \in (0, 1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 1 && x0 - x1 < 0 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousPredecessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), continuousPredecessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), continuousPredecessor.timedCondition.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousPredecessor.timedCondition.zone.value(0, 2));
    // Check the fractional order
    BOOST_REQUIRE_EQUAL(2, continuousPredecessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(3, continuousPredecessor.fractionalOrder.order.size());
    auto it = continuousPredecessor.fractionalOrder.order.begin();
    BOOST_CHECK(it->empty());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->front());

    continuousPredecessor = continuousPredecessor.predecessor();
    BOOST_CHECK_EQUAL("a", continuousPredecessor.word); // here
    // continuousPredecessor.timedCondition is \tau_0 = 1 && \tau_0 + \tau_1 \in (1, 2) && \tau_1 \in (0, 1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 <= 1 && x0 - x2 <= -1
    BOOST_CHECK_EQUAL((Bounds{1, true}), continuousPredecessor.timedCondition.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), continuousPredecessor.timedCondition.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), continuousPredecessor.timedCondition.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), continuousPredecessor.timedCondition.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), continuousPredecessor.timedCondition.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), continuousPredecessor.timedCondition.zone.value(0, 2));
    // Check the fractional order
    BOOST_REQUIRE_EQUAL(2, continuousPredecessor.fractionalOrder.size);
    BOOST_REQUIRE_EQUAL(2, continuousPredecessor.fractionalOrder.order.size());
    it = continuousPredecessor.fractionalOrder.order.begin();
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    it++;
    BOOST_CHECK_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
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
    order.order.push_back(std::deque<ClockVariables>{1});
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

  BOOST_FIXTURE_TEST_CASE(predecessor, SimpleObservationTableKeysFixture) {
    // s1 should be (epsilon, -0 <= T_{0, 0}  <= 0)
    // std::cout << s1.getTimedCondition() << std::endl;
    BOOST_CHECK_EQUAL(0, s1.wordSize());
    BOOST_CHECK_EQUAL("", s1.getWord());
    BOOST_CHECK_EQUAL((Bounds{0, true}), s1.getTimedCondition().getLowerBound(0, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s1.getTimedCondition().getUpperBound(0, 0));
    // s2 should be (a, -0 <= T_{0, 0}  <= 0 && -0 <= T_{0, 1}  <= 0 && -0 <= T_{1, 1}  <= 0)
    // std::cout << s2.getTimedCondition() << std::endl;
    BOOST_CHECK_EQUAL(1, s2.wordSize());
    BOOST_CHECK_EQUAL("a", s2.getWord());
    BOOST_CHECK_EQUAL((Bounds{0, true}), s2.getTimedCondition().getLowerBound(0, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s2.getTimedCondition().getUpperBound(0, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s2.getTimedCondition().getLowerBound(0, 1));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s2.getTimedCondition().getUpperBound(0, 1));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s2.getTimedCondition().getLowerBound(1, 1));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s2.getTimedCondition().getUpperBound(1, 1));
    // s2 should be (a, -0 < T_{0, 0}  < 1 && -0 < T_{0, 1}  < 1 && -0 <= T_{1, 1}  <= 0)
    // std::cout << s3.getTimedCondition() << std::endl;
    BOOST_CHECK_EQUAL(1, s3.wordSize());
    BOOST_CHECK_EQUAL("a", s3.getWord());
    BOOST_CHECK_EQUAL((Bounds{0, false}), s3.getTimedCondition().getLowerBound(0, 0));
    BOOST_CHECK_EQUAL((Bounds{1, false}), s3.getTimedCondition().getUpperBound(0, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), s3.getTimedCondition().getLowerBound(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), s3.getTimedCondition().getUpperBound(0, 1));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s3.getTimedCondition().getLowerBound(1, 1));
    BOOST_CHECK_EQUAL((Bounds{0, true}), s3.getTimedCondition().getUpperBound(1, 1));
  }

  BOOST_AUTO_TEST_CASE(fromWord) {
    const auto word = BackwardRegionalElementaryLanguage::fromTimedWord({"aaa", {2,1,0.5,0}});
    BOOST_CHECK_EQUAL(4, word.fractionalOrder.size);
    BOOST_CHECK_EQUAL(2, word.fractionalOrder.order.size());
    BOOST_CHECK_EQUAL(2, word.fractionalOrder.order.front().size());
    const std::vector<std::size_t> firstOrderList = {0, 1};
    BOOST_CHECK_EQUAL_COLLECTIONS(firstOrderList.begin(), firstOrderList.end(),
                                  word.fractionalOrder.order.front().begin(),
                                  word.fractionalOrder.order.front().end());
    BOOST_CHECK_EQUAL(2, word.fractionalOrder.order.back().size());
    const std::vector<std::size_t> secondOrderList = {2, 3};
    BOOST_CHECK_EQUAL_COLLECTIONS(secondOrderList.begin(), secondOrderList.end(),
                                  word.fractionalOrder.order.back().begin(),
                                  word.fractionalOrder.order.back().end());

    BOOST_CHECK_EQUAL("aaa", word.getWord());
    std::stringstream stream;
    stream << word.getTimedCondition();
    const auto expectedTimedCondition = "2 <= T_{0, 0}  <= 2 && 3 <= T_{0, 1}  <= 3 && 3 < T_{0, 2}  < 4 && 3 < T_{0, 3}  < 4 && 1 <= T_{1, 1}  <= 1 && 1 < T_{1, 2}  < 2 && 1 < T_{1, 3}  < 2 && 0 < T_{2, 2}  < 1 && 0 < T_{2, 3}  < 1 && 0 <= T_{3, 3}  <= 0";
    BOOST_CHECK_EQUAL(expectedTimedCondition, stream.str());
  }
BOOST_AUTO_TEST_SUITE_END()

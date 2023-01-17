/**
 * @author Masaki Waga
 * @date 2022/03/03.
 */
#define protected public

#include "../include/timed_condition.hh"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>

#include "simple_observation_table_keys_fixture.hh"

BOOST_AUTO_TEST_SUITE(TimedConditionTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(empty) {
    TimedCondition empty = TimedCondition::empty();
    BOOST_TEST(empty.isSimple());
  }

  BOOST_AUTO_TEST_CASE(concatenate) {
    TimedCondition left = TimedCondition::empty();
    TimedCondition right = TimedCondition::empty();

    // left is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    left.zone = Zone::top(3);
    left.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    left.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    left.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    left.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    left.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    left.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0

    // right is \tau_0 \in (0,1)
    right.zone = Zone::top(2);
    right.zone.tighten(0, -1, {1, false}); // x1 - x0 < 1
    right.zone.tighten(-1, 0, {0, false}); // x0 - x1 < 0

    TimedCondition result = left + right;
    // result should be \tau_0 \in (0,2) && \tau_0 + \tau_1 = (1,2) && \tau_1 \in (0,2)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 < 2 && x0 - x2 < 0
    BOOST_CHECK_EQUAL(2, result.size());
    BOOST_CHECK_EQUAL((Bounds{1, false}), result.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), result.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), result.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), result.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), result.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), result.zone.value(0, 2));
  }

  BOOST_AUTO_TEST_CASE(enumerate) {
    TimedCondition nonSimple;
    // nonSimple should be \tau_0 \in (0,1) && \tau_0 + \tau_1 = (1,2) && \tau_1 \in (0,2)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 < 2 && x0 - x2 < 0
    nonSimple.zone = Zone::top(3);

    nonSimple.zone.value(1, 2) = Bounds{1, false};
    nonSimple.zone.value(2, 1) = Bounds{0, false};
    nonSimple.zone.value(1, 0) = Bounds{2, false};
    nonSimple.zone.value(0, 1) = Bounds{-1, false};
    nonSimple.zone.value(2, 0) = Bounds{2, false};
    nonSimple.zone.value(0, 2) = Bounds{0, false};

    std::vector<TimedCondition> simpleConditions;
    nonSimple.enumerate(simpleConditions);
    BOOST_REQUIRE_EQUAL(3, simpleConditions.size());

    // simpleConditions[0] should be \tau_0 \in (0,1) && \tau_0 + \tau_1 = (1,2) && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_REQUIRE_EQUAL(2, simpleConditions[0].size());
    BOOST_CHECK_EQUAL((Bounds{1, false}), simpleConditions[0].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), simpleConditions[0].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), simpleConditions[0].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), simpleConditions[0].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), simpleConditions[0].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), simpleConditions[0].zone.value(0, 2));

    // simpleConditions[1] should be \tau_0 \in (0,1) && \tau_0 + \tau_1 = (1,2) && \tau_1 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 <= 1 && x0 - x2 <= -1
    BOOST_REQUIRE_EQUAL(2, simpleConditions[1].size());
    BOOST_CHECK_EQUAL((Bounds{1, false}), simpleConditions[1].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), simpleConditions[1].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), simpleConditions[1].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), simpleConditions[1].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), simpleConditions[1].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), simpleConditions[1].zone.value(0, 2));

    // simpleConditions[2] should be \tau_0 \in (0,1) && \tau_0 + \tau_1 = (1,2) && \tau_1 \in (1,2)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 < 2 && x0 - x2 < -1
    BOOST_REQUIRE_EQUAL(2, simpleConditions[2].size());
    BOOST_CHECK_EQUAL((Bounds{1, false}), simpleConditions[2].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), simpleConditions[2].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), simpleConditions[2].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), simpleConditions[2].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), simpleConditions[2].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), simpleConditions[2].zone.value(0, 2));
  }

  BOOST_AUTO_TEST_CASE(enumurate20220918Single) {
    TimedCondition condition;
    std::stringstream stream;
    condition.zone = Zone::top(2);

    condition.restrictUpperBound(0, 0, Bounds{1, true});
    condition.restrictLowerBound(0, 0, Bounds{0, false});
    stream << condition;
    BOOST_CHECK_EQUAL("-0 < T_{0, 0}  <= 1", stream.str());
    stream.str("");

    const auto enumeratedConditions = condition.enumerate();
    BOOST_CHECK_EQUAL(2, enumeratedConditions.size());
    for (const auto &enumeratedCondition: enumeratedConditions) {
      BOOST_TEST(condition.includes(enumeratedCondition));
    }
  }

  BOOST_AUTO_TEST_CASE(enumurate20220918) {
    TimedCondition condition;
    std::stringstream stream;
    condition.zone = Zone::top(3);

    condition.restrictUpperBound(0, 0, Bounds{1, false});
    condition.restrictLowerBound(0, 0, Bounds{0, false});
    condition.restrictUpperBound(0, 1, Bounds{2, false});
    condition.restrictLowerBound(0, 1, Bounds{-1, false});
    condition.restrictUpperBound(1, 1, Bounds{1, true});
    condition.restrictLowerBound(1, 1, Bounds{0, false});
    stream << condition;
    BOOST_CHECK_EQUAL("-0 < T_{0, 0}  < 1 && 1 < T_{0, 1}  < 2 && -0 < T_{1, 1}  <= 1", stream.str());
    stream.str("");

    const auto enumeratedConditions = condition.enumerate();
    BOOST_CHECK_EQUAL(2, enumeratedConditions.size());
    for (const auto &enumeratedCondition: enumeratedConditions) {
      BOOST_TEST(condition.includes(enumeratedCondition));
    }
  }

  BOOST_AUTO_TEST_CASE(succesor) {
    TimedCondition condition;
    // condition is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    condition.zone = Zone::top(3);
    condition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    condition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    condition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    condition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    condition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    condition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0

    std::vector<TimedCondition> successors = {condition.successor({0}), condition.successor({1}),
                                              condition.successor({0, 1})};
    for (const auto &successor: successors) {
      BOOST_REQUIRE_EQUAL(2, successor.size());
    }

    // successors[0] is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (1, 2) && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0  && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_CHECK_EQUAL((Bounds{1, false}), successors[0].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), successors[0].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), successors[0].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), successors[0].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), successors[0].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), successors[0].zone.value(0, 2));

    // successors[1] is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 <= 1 && x0 - x2 <= -1
    BOOST_CHECK_EQUAL((Bounds{1, false}), successors[1].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), successors[1].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), successors[1].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), successors[1].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), successors[1].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), successors[1].zone.value(0, 2));

    // successors[2] is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (1, 2) && \tau_1 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 < 2 && x0 - x1 < -1 && x2 - x0 <= 1 && x0 - x2 <= -1
    BOOST_CHECK_EQUAL((Bounds{1, false}), successors[2].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), successors[2].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), successors[2].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), successors[2].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), successors[2].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), successors[2].zone.value(0, 2));
  }

  BOOST_AUTO_TEST_CASE(predecessor) {
    TimedCondition condition;
    // condition is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    condition.zone = Zone::top(3);
    condition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    condition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    condition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    condition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    condition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    condition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0

    std::vector<TimedCondition> predecessors = {condition.predecessor({0}), condition.predecessor({1}),
                                                condition.predecessor({0, 1})};
    for (const auto &predecessor: predecessors) {
      BOOST_REQUIRE_EQUAL(2, predecessor.size());
    }

    // predecessors[0] is \tau_0 = 1 && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 <= 1 && x2 - x1 <= -1  && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_CHECK_EQUAL((Bounds{1, true}), predecessors[0].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), predecessors[0].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), predecessors[0].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), predecessors[0].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), predecessors[0].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), predecessors[0].zone.value(0, 2));

    // predecessors[1] is \tau_0 \in (0,1) && \tau_0 + \tau_1 \in (1, 2) && \tau_1 \in (0, 1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_CHECK_EQUAL((Bounds{1, false}), predecessors[1].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), predecessors[1].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), predecessors[1].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), predecessors[1].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), predecessors[1].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), predecessors[1].zone.value(0, 2));

    // predecessors[2] is \tau_0 = 1 && \tau_0 + \tau_1 \in (1, 2) && \tau_1 \in (0, 1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 <= 1 && x2 - x1 <= 1 && x1 - x0 < 2 && x0 - x1 < 1 && x2 - x0 < 1 && x0 - x2 < 0
    BOOST_CHECK_EQUAL((Bounds{1, true}), predecessors[2].zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), predecessors[2].zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{2, false}), predecessors[2].zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, false}), predecessors[2].zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), predecessors[2].zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), predecessors[2].zone.value(0, 2));
  }

  BOOST_AUTO_TEST_CASE(extendN) {
    TimedCondition condition;
    // condition is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    condition.zone = Zone::top(3);
    condition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    condition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    condition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    condition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    condition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    condition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0

    auto extendN = condition.extendN();
    BOOST_REQUIRE_EQUAL(3, extendN.size());
    // extendN is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1) && \tau_0 + \tau_1 + \tau_2 = 1 && \tau_1 + \tau_2 \in (0,1) && \tau_2 = 0
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1 + \tau_2, x2 == \tau_1 + \tau_2, and x3 == \tau_2.
    // Therefore, we have
    //   - x1 - x2 < 1 && x2 - x1 < 0 &&
    //   - x1 - x3 <= 1 && x3 - x1 <= -1 &&
    //   - x2 - x3 < 1 && x3 - x2 < 0 &&
    //   - x1 - x0 <= 1 && x0 - x1 <= -1 &&
    //   - x2 - x0 < 1 && x0 - x2 < 0 &&
    //   - x3 - x0 <= 0 && x0 - x3 <= 0
    BOOST_CHECK_EQUAL((Bounds{1, false}), extendN.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, false}), extendN.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), extendN.zone.value(1, 3));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), extendN.zone.value(3, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), extendN.zone.value(2, 3));
    BOOST_CHECK_EQUAL((Bounds{0, false}), extendN.zone.value(3, 2));
    BOOST_CHECK_EQUAL((Bounds{1, true}), extendN.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), extendN.zone.value(0, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), extendN.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), extendN.zone.value(0, 2));
    BOOST_CHECK_EQUAL((Bounds{0, true}), extendN.zone.value(3, 0));
    BOOST_CHECK_EQUAL((Bounds{0, true}), extendN.zone.value(0, 3));
  }

  BOOST_AUTO_TEST_CASE(extendZero) {
    TimedCondition condition;
    // condition is \tau_0 \in (0,1) && \tau_0 + \tau_1 = 1 && \tau_1 \in (0,1)
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1, and x2 == \tau_1.
    // Therefore, we have x1 - x2 < 1 && x2 - x1 < 0 && x1 - x0 <= 1 && x0 - x1 <= -1 && x2 - x0 < 1 && x0 - x2 < 0
    condition.zone = Zone::top(3);
    condition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
    condition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
    condition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    condition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    condition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
    condition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0

    auto extendZero = condition.extendZero();
    BOOST_REQUIRE_EQUAL(3, extendZero.size());

    // extendZero is
    //   - \tau_1 \in (0,1) &&
    //   - \tau_1 + \tau_2 = 1 &&
    //   - \tau_2 \in (0,1) &&
    //   - \tau_0 = 0 &&
    //   - \tau_0 + \tau_1 \in (0,1) &&
    //   - \tau_0 + \tau_1 + \tau_2 = 1
    // Our encoding is x0 == 0, x1 == \tau_0 + \tau_1 + \tau_2, x2 == \tau_1 + \tau_2, and x3 == \tau_2.
    // Therefore, we have
    //   - x2 - x3 < 1 && x3 - x2 < 0 &&
    //   - x2 - x0 <= 1 && x0 - x2 <= -1 &&
    //   - x3 - x0 < 1 && x0 - x3 < 0 &&
    //   - x1 - x2 <= 0 && x2 - x1 <= 0
    //   - x1 - x3 < 1 && x3 - x1 < 0 &&
    //   - x1 - x0 <= 1 && x0 - x1 <= -1 &&
    BOOST_CHECK_EQUAL((Bounds{1, false}), extendZero.zone.value(2, 3));
    BOOST_CHECK_EQUAL((Bounds{0, false}), extendZero.zone.value(3, 2));
    BOOST_CHECK_EQUAL((Bounds{1, true}), extendZero.zone.value(2, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), extendZero.zone.value(0, 2));
    BOOST_CHECK_EQUAL((Bounds{1, false}), extendZero.zone.value(3, 0));
    BOOST_CHECK_EQUAL((Bounds{0, false}), extendZero.zone.value(0, 3));
    BOOST_CHECK_EQUAL((Bounds{0, true}), extendZero.zone.value(1, 2));
    BOOST_CHECK_EQUAL((Bounds{0, true}), extendZero.zone.value(2, 1));
    BOOST_CHECK_EQUAL((Bounds{1, false}), extendZero.zone.value(1, 3));
    BOOST_CHECK_EQUAL((Bounds{0, false}), extendZero.zone.value(3, 1));
    BOOST_CHECK_EQUAL((Bounds{1, true}), extendZero.zone.value(1, 0));
    BOOST_CHECK_EQUAL((Bounds{-1, true}), extendZero.zone.value(0, 1));
  }

  BOOST_FIXTURE_TEST_CASE(p1s1Juxtaposition, SimpleObservationTableKeysFixture) {
    auto juxtaposition = p1.getTimedCondition() ^ s1.getTimedCondition();
    juxtaposition.canonize();

    BOOST_CHECK(juxtaposition.isSatisfiable());
  }

  BOOST_AUTO_TEST_CASE(convexHullTest) {
    TimedCondition left, right;
    std::stringstream stream;
    left.zone = Zone::top(2);
    right.zone = Zone::top(2);

    left.zone.tighten(-1, 0, {0, true}); // x0 <= 0
    left.zone.tighten(0, -1, {0, true}); // x0 >= 0
    stream << left;
    BOOST_CHECK_EQUAL("-0 <= T_{0, 0}  <= 0", stream.str());
    stream.str("");

    right.zone.tighten(0, -1, {1, false}); // x0 < 1
    right.zone.tighten(-1, 0, {0, false}); // x0 > 0
    stream << right;
    BOOST_CHECK_EQUAL("-0 < T_{0, 0}  < 1", stream.str());
    stream.str("");

    stream << left.convexHull(right);
    BOOST_CHECK_EQUAL("-0 <= T_{0, 0}  < 1", stream.str());
  }

  BOOST_AUTO_TEST_CASE(applyResetsTest) {
    std::stringstream stream;
    TimedCondition condition {Zone::top(4)};
    condition.restrictLowerBound(0, 0, Bounds{-6, false});
    condition.restrictUpperBound(0, 0, Bounds{7, false});
    condition.restrictLowerBound(0, 1, Bounds{-8, true});
    condition.restrictUpperBound(0, 1, Bounds{8, true});
    condition.restrictLowerBound(0, 2, Bounds{-12, false});
    condition.restrictUpperBound(0, 2, Bounds{13, false});
    condition.restrictLowerBound(1, 1, Bounds{-1, false});
    condition.restrictUpperBound(1, 1, Bounds{2, false});
    condition.restrictLowerBound(1, 2, Bounds{-6, false});
    condition.restrictUpperBound(1, 2, Bounds{7, false});
    condition.restrictLowerBound(2, 2, Bounds{-4, false});
    condition.restrictUpperBound(2, 2, Bounds{5, false});
    stream << condition;
    BOOST_CHECK_EQUAL(stream.str(),
                      "6 < T_{0, 0}  < 7 && 8 <= T_{0, 1}  <= 8 && 12 < T_{0, 2}  < 13 && 1 < T_{1, 1}  < 2 && 6 < T_{1, 2}  < 7 && 4 < T_{2, 2}  < 5");
    stream.str("");

    TATransition::Resets resets;
    resets.emplace_back(static_cast<ClockVariables>(1), 5.75);
    resets.emplace_back(static_cast<ClockVariables>(3), 0.0);
    const auto resetCondition = condition.extendN().applyResets(resets);
    stream << resetCondition;
    BOOST_CHECK_EQUAL(stream.str(),
                      "6 < T_{0, 0}  < 8 && 8 <= T_{0, 1}  <= 8 && 12 < T_{0, 2}  < 13 && 12 < T_{0, 3}  < 13 && -0 < T_{1, 1}  < 2 && 5 < T_{1, 2}  < 6 && 5 < T_{1, 3}  < 6 && 4 < T_{2, 2}  < 5 && 4 < T_{2, 3}  < 5 && 0 <= T_{3, 3}  <= 0");
    stream.str("");
  }

  BOOST_AUTO_TEST_CASE(applyResetsTest20230111) {
    std::stringstream stream;
    TimedCondition condition {Zone::top(4)};
    condition.restrictLowerBound(0, 0, Bounds{-6, false});
    condition.restrictUpperBound(0, 0, Bounds{7, false});
    condition.restrictLowerBound(0, 1, Bounds{-8, true});
    condition.restrictUpperBound(0, 1, Bounds{8, true});
    condition.restrictLowerBound(0, 2, Bounds{-12, false});
    condition.restrictUpperBound(0, 2, Bounds{13, false});
    condition.restrictLowerBound(1, 1, Bounds{-1, false});
    condition.restrictUpperBound(1, 1, Bounds{2, false});
    condition.restrictLowerBound(1, 2, Bounds{-6, false});
    condition.restrictUpperBound(1, 2, Bounds{7, false});
    condition.restrictLowerBound(2, 2, Bounds{-4, false});
    condition.restrictUpperBound(2, 2, Bounds{5, false});
    stream << condition;
    BOOST_CHECK_EQUAL(stream.str(),
                      "6 < T_{0, 0}  < 7 && 8 <= T_{0, 1}  <= 8 && 12 < T_{0, 2}  < 13 && 1 < T_{1, 1}  < 2 && 6 < T_{1, 2}  < 7 && 4 < T_{2, 2}  < 5");
    stream.str("");

    TATransition::Resets resets;
    resets.emplace_back(static_cast<ClockVariables>(1), 5.75);
    resets.emplace_back(static_cast<ClockVariables>(3), 0.0);
    const auto resetCondition = condition.applyResets(resets, 4);
    stream << resetCondition;
    BOOST_CHECK_EQUAL(stream.str(),
                      "6 < T_{0, 0}  < 8 && 8 <= T_{0, 1}  <= 8 && 12 < T_{0, 2}  < 13 && 12 < T_{0, 3}  < 13 && -0 < T_{1, 1}  < 2 && 5 < T_{1, 2}  < 6 && 5 < T_{1, 3}  < 6 && 4 < T_{2, 2}  < 5 && 4 < T_{2, 3}  < 5 && 0 <= T_{3, 3}  <= 0");
    stream.str("");
  }

  BOOST_AUTO_TEST_CASE(applyResetsTest20230117) {
    std::stringstream stream;
    TimedCondition original {Zone::top(3)};
    original.restrictUpperBound(0, 0, Bounds{1, false});
    original.restrictLowerBound(0, 0, Bounds{0, false});
    original.restrictUpperBound(0, 1, Bounds{6, false});
    original.restrictLowerBound(0, 1, Bounds{-5, false});
    original.restrictUpperBound(1, 1, Bounds{6, false});
    original.restrictLowerBound(1, 1, Bounds{-5, false});
    stream << original;
    BOOST_CHECK_EQUAL("-0 < T_{0, 0}  < 1 && 5 < T_{0, 1}  < 6 && 5 < T_{1, 1}  < 6", stream.str());
    stream.str("");
    TATransition::Resets  resets;
    resets.emplace_back(0, 1.5);
    resets.emplace_back(1, 1.25);
    resets.emplace_back(2, 0.0);
    stream << resets;
    BOOST_CHECK_EQUAL("x0 := 1.5, x1 := 1.25, x2 := 0", stream.str());
    stream.str("");

    auto result = original.applyResets(resets, 3);
    // Since the clock valuation after the given reset is unique, the result should be simple.
    BOOST_TEST(result.isSimple());

    // The reset value must be satisfiable at least
    result.restrictUpperBound(0, 2, Bounds{1.5, true});
    result.restrictLowerBound(0, 2, Bounds{-1.5, true});
    result.restrictUpperBound(1, 2, Bounds{1.25, true});
    result.restrictLowerBound(1, 2, Bounds{-1.25, true});
    result.restrictUpperBound(2, 2, Bounds{0, true});
    result.restrictLowerBound(2, 2, Bounds{-0, true});
    BOOST_TEST(result.zone.isSatisfiable());
  }
BOOST_AUTO_TEST_SUITE_END()

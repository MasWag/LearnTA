/**
 * @author Masaki Waga
 * @date 2022/09/14.
 */
#include <boost/test/unit_test.hpp>

#define protected public
#define private public

#include "internal_transition_maker.hh"

BOOST_AUTO_TEST_SUITE(InternalTransitionMakerTest)

  using namespace learnta;
  BOOST_AUTO_TEST_SUITE(make)

    BOOST_AUTO_TEST_CASE(single) {
      InternalTransitionMaker maker;
      const auto target = std::make_shared<TAState>(false);
      TimedCondition condition = TimedCondition::empty();
      condition.zone = Zone::top(3);
      condition.zone.tighten(0, 1, {1, false}); // x1 - x2 < 1
      condition.zone.tighten(1, 0, {0, false}); // x2 - x1 < 0
      condition.zone.tighten(0, -1, {1, true}); // x1 - x0 <= 1
      condition.zone.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
      condition.zone.tighten(1, -1, {1, false}); // x2 - x0 < 1
      condition.zone.tighten(-1, 1, {0, false}); // x0 - x2 < 0
      std::stringstream stream;
      stream << condition;
      BOOST_CHECK_EQUAL("-0 < T_{0, 0}  < 1 && 1 <= T_{0, 1}  <= 1 && -0 < T_{1, 1}  < 1", stream.str());
      maker.add(target, condition);

      BOOST_CHECK_EQUAL(1, maker.size());

      const auto result = maker.make();
      BOOST_CHECK_EQUAL(1, result.size());
      BOOST_CHECK_EQUAL(target.get(), result.front().target);
      BOOST_CHECK_EQUAL(1, result.front().resetVars.size());
      BOOST_CHECK_EQUAL(2, result.front().resetVars.front().first);
      BOOST_CHECK_EQUAL(0, result.front().resetVars.front().second.index());
      BOOST_CHECK_EQUAL(0.0, std::get<double>(result.front().resetVars.front().second));
      std::vector<Constraint> expectedGuard = {ConstraintMaker(0) >= 1, ConstraintMaker(0) <= 1,
                                               ConstraintMaker(1) > 0, ConstraintMaker(1) < 1};
      BOOST_CHECK_EQUAL(expectedGuard, result.front().guard);
    }

  BOOST_AUTO_TEST_SUITE_END()

  BOOST_AUTO_TEST_SUITE(toReset)

    BOOST_AUTO_TEST_CASE(SmallPC20220915) {
      TimedCondition condition = TimedCondition::empty();
      condition.zone = Zone::top(5);
      condition.restrictUpperBound(0, 0, {1, false});
      condition.restrictLowerBound(0, 0, {0, false});
      condition.restrictUpperBound(0, 1, {1, false});
      condition.restrictLowerBound(0, 1, {0, false});
      condition.restrictUpperBound(0, 2, {2, true});
      condition.restrictLowerBound(0, 2, {-2, true});
      condition.restrictUpperBound(0, 3, {2, true});
      condition.restrictLowerBound(0, 3, {-2, true});
      condition.restrictUpperBound(1, 1, {0, true});
      condition.restrictLowerBound(1, 1, {0, true});
      condition.restrictUpperBound(1, 2, {2, false});
      condition.restrictLowerBound(1, 2, {-1, false});
      condition.restrictUpperBound(1, 3, {2, false});
      condition.restrictLowerBound(1, 3, {-1, false});
      condition.restrictUpperBound(2, 2, {2, false});
      condition.restrictLowerBound(2, 2, {-1, false});
      condition.restrictUpperBound(2, 3, {2, false});
      condition.restrictLowerBound(2, 3, {-1, false});
      condition.restrictUpperBound(3, 3, {0, true});
      condition.restrictLowerBound(3, 3, {0, true});


      std::stringstream stream;
      stream << condition;
      BOOST_CHECK_EQUAL("-0 < T_{0, 0}  < 1 && -0 < T_{0, 1}  < 1 && 2 <= T_{0, 2}  <= 2 && 2 <= T_{0, 3}  <= 2 && -0 <= T_{1, 1}  <= 0 && 1 < T_{1, 2}  < 2 && 1 < T_{1, 3}  < 2 && 1 < T_{2, 2}  < 2 && 1 < T_{2, 3}  < 2 && -0 <= T_{3, 3}  <= 0", stream.str());
      const auto resets = InternalTransitionMaker::toReset(condition);

      BOOST_CHECK_EQUAL(4, resets.size());

      BOOST_CHECK_EQUAL(0, resets.at(0).first);
      BOOST_CHECK_EQUAL(0, resets.at(0).second.index());
      BOOST_CHECK_EQUAL(2.0, std::get<double>(resets.at(0).second));
      BOOST_CHECK_EQUAL(1, resets.at(1).first);
      BOOST_CHECK_EQUAL(0, resets.at(1).second.index());
      BOOST_CHECK_EQUAL(1.5, std::get<double>(resets.at(1).second));
      BOOST_CHECK_EQUAL(2, resets.at(2).first);
      BOOST_CHECK_EQUAL(0, resets.at(2).second.index());
      BOOST_CHECK_EQUAL(1.5, std::get<double>(resets.at(2).second));
      BOOST_CHECK_EQUAL(3, resets.at(3).first);
      BOOST_CHECK_EQUAL(0, resets.at(3).second.index());
      BOOST_CHECK_EQUAL(0, std::get<double>(resets.at(3).second));
    }

  BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

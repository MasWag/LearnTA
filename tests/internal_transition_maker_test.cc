/**
 * @author Masaki Waga
 * @date 2022/09/14.
 */
#include <boost/test/unit_test.hpp>

#define protected public

#include "internal_transition_maker.hh"

BOOST_AUTO_TEST_SUITE(InternalTransitionMakerTest)

  using namespace learnta;

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

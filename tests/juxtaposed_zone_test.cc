/**
 * @author Masaki Waga
 * @date 2022/03/14.
 */


#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../include/juxtaposed_zone.hh"

#include "simple_observation_table_keys_fixture.hh"

BOOST_AUTO_TEST_SUITE(JuxtaposedZoneTest)

  using namespace learnta;
  BOOST_AUTO_TEST_CASE(juxtaposeWithoutCommonVariables) {
    Zone left = Zone::top(3);
    left.tighten(0, 1, {1, false}); // x1 - x2 < 1
    left.tighten(1, 0, {0, false}); // x2 - x1 < 0
    left.tighten(0, -1, {1, true}); // x1 - x0 <= 1
    left.tighten(-1, 0, {-1, true}); // x0 - x1 <= -1
    left.tighten(1, -1, {1, false}); // x2 - x0 < 1
    left.tighten(-1, 1, {0, false}); // x0 - x2 < 0

    // right is \tau_0 \in (0,1)
    Zone right = Zone::top(2);
    right.tighten(0, -1, {1, false}); // x1 - x0 < 1
    right.tighten(-1, 0, {0, false}); // x0 - x1 < 0

    BOOST_CHECK_EQUAL((JuxtaposedZone{left, right}), (JuxtaposedZone{left, right, 0}));
  }

  BOOST_FIXTURE_TEST_CASE(juxtaposeSymbolicMembership, SimpleObservationTableKeysFixture) {
    std::cout << p1.getTimedCondition() << std::endl;
    std::cout << p2.getTimedCondition() << std::endl;
    std::cout << s2.getTimedCondition() << std::endl;
    std::cout << s3.getTimedCondition() << std::endl;
    auto p1s3 = p1 + s3;
    std::cout << p1s3.getTimedCondition() << std::endl;
    auto p2s3 = p2 + s3;
    std::cout << p2s3.getTimedCondition() << std::endl;
  }
BOOST_AUTO_TEST_SUITE_END()
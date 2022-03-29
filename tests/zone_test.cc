#include <boost/test/unit_test.hpp>

#include "../include/zone.hh"

BOOST_AUTO_TEST_SUITE(ZoneTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(makeUnsat) {
    Zone zone = Zone::zero(5);
    for (int i = 0; i < 5; ++i) {
      for (int j = 0; j < 5; ++j) {
        zone.value(i, j) = {i * j, true};
      }
    }

    BOOST_TEST(zone.isSatisfiable());
    zone.makeUnsat();
    BOOST_TEST(!zone.isSatisfiable());
  }

  BOOST_AUTO_TEST_CASE(top) {
    Zone top = Zone::top(10);
    BOOST_TEST(top.isSatisfiable());
  }
/*
BOOST_AUTO_TEST_CASE(makeGuard) {
  Zone zone = Zone::zero(5);
  zone.M = {10000, true};
  zone.elapse();
  
  std::vector<Constraint> guard = zone.makeGuard();
  BOOST_CHECK_EQUAL(4, guard.size());
  const std::vector<ClockVariables> expectedX = {0, 1, 2, 3};
  const std::vector<Constraint::Order> expectedOdr =
    {Constraint::Order::gt,
     Constraint::Order::gt,
     Constraint::Order::gt,
     Constraint::Order::gt};
  const std::vector<int> expectedC = {0, 0, 0, 0};
  for (int i = 0; i < 4; ++i) {
    BOOST_CHECK_EQUAL(expectedX[i], guard[i].x);
    BOOST_CHECK_EQUAL(expectedOdr[i], guard[i].odr);
    BOOST_CHECK_EQUAL(expectedC[i], guard[i].c);
  }
}
*/

  BOOST_AUTO_TEST_CASE(elapseTighten) {
    const auto guard = ConstraintMaker(0) >= 5;
    // Construct the initial zone
    auto zone = Zone::top(5);
    zone.reset(0);
    zone.reset(2);
    zone.tighten(ConstraintMaker(1) >= 0);
    zone.tighten(ConstraintMaker(3) >= 0);
    zone.tighten(1, 3, Bounds{0, true});
    zone.tighten(3, 1, Bounds{0, true});
    zone.canonize();
    std::cout << zone << std::endl;
    BOOST_TEST(zone.isSatisfiable());

    // Time elapse
    zone.elapse();
    std::cout << zone << std::endl;
    BOOST_TEST(zone.isSatisfiable());

    // Guard
    zone.tighten(guard);
    std::cout << zone << std::endl;
    BOOST_TEST(zone.isSatisfiable());
  }
BOOST_AUTO_TEST_SUITE_END()

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

  BOOST_AUTO_TEST_CASE(elapseTighten20220902) {
    /*
(0, true) (-1, false) (-1, false) (-1, false) (-1, false) (0, true) (-1, false) (-1, false) (0, true) (-1, false)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (0, true) (0, true) (1.79769e+308, false) (0, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (0, true) (0, true) (1.79769e+308, false) (0, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (0, true) (0, true) (1.79769e+308, false) (0, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (0, true) (0, true) (1.79769e+308, false) (0, true)
(1.79769e+308, false) (-1, false) (-1, false) (-1, false) (-1, false) (0, true) (-1, false) (-1, false) (1.79769e+308, false) (-1, false)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (0, true) (0, true) (1.79769e+308, false) (0, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (0, true) (0, true) (1.79769e+308, false) (0, true)
(0, true) (-1, false) (-1, false) (-1, false) (-1, false) (0, true) (-1, false) (-1, false) (0, true) (-1, false)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (0, true) (0, true) (1.79769e+308, false) (0, true)
     */
    // Construct the initial zone
    auto zone = Zone::top(10);
    zone.M = Bounds{INT_MAX, true};
    zone.value(1, 2) = Bounds{0.0, true};
    zone.value(2, 1) = Bounds{0.0, true};
    zone.value(1, 3) = Bounds{0.0, true};
    zone.value(3, 1) = Bounds{0.0, true};
    zone.value(1, 4) = Bounds{0.0, true};
    zone.value(4, 1) = Bounds{0.0, true};
    zone.value(1, 6) = Bounds{0.0, true};
    zone.value(6, 1) = Bounds{0.0, true};
    zone.value(1, 7) = Bounds{0.0, true};
    zone.value(7, 1) = Bounds{0.0, true};
    zone.value(1, 9) = Bounds{0.0, true};
    zone.value(9, 1) = Bounds{0.0, true};
    zone.value(0, 8) = Bounds{0.0, true};
    zone.value(8, 0) = Bounds{0.0, true};
    zone.value(1, 5) = Bounds{2.0, false};
    zone.value(5, 1) = Bounds{-1, false};
    zone.value(0, 5) = Bounds{0, true};
    zone.value.diagonal().fill(Bounds{0, true});

    zone.canonize();
    zone.tighten(ConstraintMaker(0) > 1);
    std::cout << zone << std::endl;

    // time elapse
    zone.elapse();
    std::cout << zone << std::endl;

    // Tighten the zone with guards
    zone.tighten(ConstraintMaker(2) >= 1);
    zone.tighten(ConstraintMaker(3) >= 1);
    std::cout << zone << std::endl;

    // Reset variables
    zone.unconstrain(5);
    zone.value(5 + 1, 4 + 1) = Bounds{0.0, true};
    zone.value(4 + 1, 5 + 1) = Bounds{0.0, true};
    zone.unconstrain(6);
    zone.value(6 + 1, 4 + 1) = Bounds{0.0, true};
    zone.value(4 + 1, 6 + 1) = Bounds{0.0, true};
    zone.unconstrain(7);
    zone.value(7 + 1, 4 + 1) = Bounds{0.0, true};
    zone.value(4 + 1, 7 + 1) = Bounds{0.0, true};
    std::cout << zone << std::endl;

    zone.canonize();
    zone.abstractize();
    zone.value.diagonal().fill(Bounds{0, true});

    std::cout << zone << std::endl;

    /*
x2 >= 1, x3 >= 1, x5 := x4, x6 := x4, x7 := x4,
(0, true) (-1, true) (-1, true) (-1, true) (-1, true) (0, true) (0, true) (0, true) (0, true) (-1, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (2, false) (2, false) (2, false) (0, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (2, false) (2, false) (2, false) (0, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (2, false) (2, false) (2, false) (0, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (2, false) (2, false) (2, false) (0, true)
(1.79769e+308, false) (-1, true) (-1, true) (-1, true) (-1, true) (0, true) (0, true) (0, true) (0, true) (-1, true)
(1.79769e+308, false) (-1, true) (-1, true) (-1, true) (-1, true) (0, true) (0, true) (0, true) (0, true) (-1, true)
(1.79769e+308, false) (-1, true) (-1, true) (-1, true) (-1, true) (0, true) (0, true) (0, true) (0, true) (-1, true)
(1.79769e+308, false) (-1, true) (-1, true) (-1, true) (-1, true) (0, true) (0, true) (0, true) (0, true) (-1, true)
(1.79769e+308, false) (0, true) (0, true) (0, true) (0, true) (2, false) (2, false) (2, false) (2, false) (0, true)
     */
  }

  BOOST_AUTO_TEST_CASE(equalTighten) {
    // Construct the initial zone
    auto zone = Zone::top(2);
    zone.tighten(ConstraintMaker(0) > 1);
    zone.tighten(ConstraintMaker(0) < 5);
    zone.canonize();
    BOOST_TEST((Bounds{-1, false} == zone.value(0, 1)));
    BOOST_TEST((Bounds{5, false} == zone.value(1, 0)));
    zone.tighten(-1, 0, Bounds{-1, true});
    BOOST_TEST((Bounds{-1, false} == zone.value(0, 1)));
    zone.tighten(ConstraintMaker(0) >= 1);
    BOOST_TEST((Bounds{-1, false} == zone.value(0, 1)));
  }
BOOST_AUTO_TEST_SUITE_END()

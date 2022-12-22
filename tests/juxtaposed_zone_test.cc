/**
 * @author Masaki Waga
 * @date 2022/03/14.
 */


#include <boost/test/unit_test.hpp>
#include <sstream>
#define private public
#define protected public
#include "../include/juxtaposed_zone.hh"
#include "../include/renaming_relation.hh"

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

  BOOST_FIXTURE_TEST_CASE(p2p13Juxtapose, SimpleObservationTableKeysFixture) {
    auto juxtaposition = p2.getTimedCondition() ^ p13.getTimedCondition();
    BOOST_CHECK_EQUAL(1, juxtaposition.leftSize);
    BOOST_CHECK_EQUAL(3, juxtaposition.rightSize);
    std::cout << juxtaposition << std::endl;
    BOOST_CHECK(juxtaposition.isSatisfiable());
    // tau_0 + tau'_0 in p2 is equal to \tau_1 + \tau_2 + \tau'_0 in p13
    RenamingRelation renaming = {{std::make_pair(0, 1)}};
    juxtaposition.addRenaming(renaming);
    std::cout << juxtaposition << std::endl;
    BOOST_CHECK(juxtaposition.isSatisfiable());
    BOOST_TEST(renaming == juxtaposition.makeRenaming(), boost::test_tools::per_element());
  }

  BOOST_AUTO_TEST_CASE(makeImplicitRenaming) {
    std::stringstream stream;
    // domain: (ab, 1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 1 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1)
    TimedCondition domain {Zone::top(4)};
    domain.restrictLowerBound(0, 0, Bounds{-1, false});
    domain.restrictUpperBound(0, 0, Bounds{2, false});
    domain.restrictLowerBound(0, 1, Bounds{-2, true});
    domain.restrictUpperBound(0, 1, Bounds{2, true});
    domain.restrictLowerBound(0, 2, Bounds{-2, false});
    domain.restrictUpperBound(0, 2, Bounds{3, false});
    domain.restrictLowerBound(1, 1, Bounds{0, false});
    domain.restrictUpperBound(1, 1, Bounds{1, false});
    domain.restrictLowerBound(1, 2, Bounds{0, false});
    domain.restrictUpperBound(1, 2, Bounds{1, false});
    domain.restrictLowerBound(2, 2, Bounds{0, false});
    domain.restrictUpperBound(2, 2, Bounds{1, false});
    stream << domain;
    BOOST_CHECK_EQUAL("1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 1 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1", stream.str());
    stream.str("");

    // codomain: (ab, 2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 <= T_{1, 1}  <= 0 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1)
    TimedCondition codomain {Zone::top(4)};
    codomain.restrictLowerBound(0, 0, Bounds{-2, true});
    codomain.restrictUpperBound(0, 0, Bounds{2, true});
    codomain.restrictLowerBound(0, 1, Bounds{-2, true});
    codomain.restrictUpperBound(0, 1, Bounds{2, true});
    codomain.restrictLowerBound(0, 2, Bounds{-2, false});
    codomain.restrictUpperBound(0, 2, Bounds{3, false});
    codomain.restrictLowerBound(1, 1, Bounds{0, true});
    codomain.restrictUpperBound(1, 1, Bounds{0, true});
    codomain.restrictLowerBound(1, 2, Bounds{0, false});
    codomain.restrictUpperBound(1, 2, Bounds{1, false});
    codomain.restrictLowerBound(2, 2, Bounds{0, false});
    codomain.restrictUpperBound(2, 2, Bounds{1, false});
    stream << codomain;
    BOOST_CHECK_EQUAL("2 <= T_{0, 0}  <= 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 <= T_{1, 1}  <= 0 && -0 < T_{1, 2}  < 1 && -0 < T_{2, 2}  < 1", stream.str());
    stream.str("");

    // renaming: {t0 == t'0}
    RenamingRelation renaming;
    renaming.emplace_back(0, 0);
    stream << renaming;
    BOOST_CHECK_EQUAL("{t0 == t'0}", stream.str());
    stream.str("");

    auto juxtaposition = codomain ^ domain;
    juxtaposition.addRenaming(renaming);
    RenamingRelation expectedRenaming;
    expectedRenaming.emplace_back(0, 0);
    expectedRenaming.emplace_back(1, 2);
    expectedRenaming.emplace_back(2, 2);
    BOOST_TEST(expectedRenaming == juxtaposition.makeRenaming(), boost::test_tools::per_element());
  }
BOOST_AUTO_TEST_SUITE_END()
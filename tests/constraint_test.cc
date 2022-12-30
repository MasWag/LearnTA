/**
 * @author Masaki Waga
 * @date 2022/09/05.
 */

#include "constraint.hh"

using namespace learnta;

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(ConstraintTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(negateAllTest) {
    std::vector<std::vector<Constraint>> dnfConstraints;
    dnfConstraints.resize(5);
    dnfConstraints.at(0).push_back(ConstraintMaker(0) >= 2);
    dnfConstraints.at(0).push_back(ConstraintMaker(1) <= 0);
    dnfConstraints.at(1).push_back(ConstraintMaker(0) > 2);
    dnfConstraints.at(1).push_back(ConstraintMaker(1) > 0);
    dnfConstraints.at(1).push_back(ConstraintMaker(1) < 1);
    dnfConstraints.at(2).push_back(ConstraintMaker(0) >= 3);
    dnfConstraints.at(2).push_back(ConstraintMaker(1) >= 1);
    dnfConstraints.at(2).push_back(ConstraintMaker(1) <= 1);
    dnfConstraints.at(3).push_back(ConstraintMaker(0) > 3);
    dnfConstraints.at(3).push_back(ConstraintMaker(1) > 1);
    dnfConstraints.at(3).push_back(ConstraintMaker(1) < 2);
    dnfConstraints.at(4).push_back(ConstraintMaker(0) >= 4);
    dnfConstraints.at(4).push_back(ConstraintMaker(1) >= 2);
    dnfConstraints.at(4).push_back(ConstraintMaker(1) <= 2);
    std::vector<std::vector<Constraint>> expected;
    expected.resize(dnfConstraints.size());
    expected.at(0) = {ConstraintMaker(0) < 2, ConstraintMaker(1) > 0};
    expected.at(1) = {ConstraintMaker(0) <= 2, ConstraintMaker(1) <= 0, ConstraintMaker(1) >= 1};
    expected.at(2) = {ConstraintMaker(0) < 3, ConstraintMaker(1) < 1, ConstraintMaker(1) > 1};
    expected.at(3) = {ConstraintMaker(0) <= 3, ConstraintMaker(1) <= 1, ConstraintMaker(1) >= 2};
    expected.at(4) = {ConstraintMaker(0) < 4, ConstraintMaker(1) < 2, ConstraintMaker(1) > 2};

    BOOST_CHECK_EQUAL(expected.size(), dnfConstraints.size());
    for (int i = 0; i < dnfConstraints.size(); ++i) {
      BOOST_CHECK_EQUAL(expected.at(i), negateAll(dnfConstraints.at(i)));
    }
  }

  BOOST_AUTO_TEST_CASE(negateSet) {
    std::vector<std::vector<Constraint>> dnfConstraints;
    dnfConstraints.resize(5);
    dnfConstraints.at(0).push_back(ConstraintMaker(0) >= 2);
    dnfConstraints.at(0).push_back(ConstraintMaker(1) <= 0);
    dnfConstraints.at(1).push_back(ConstraintMaker(0) > 2);
    dnfConstraints.at(1).push_back(ConstraintMaker(1) > 0);
    dnfConstraints.at(1).push_back(ConstraintMaker(1) < 1);
    dnfConstraints.at(2).push_back(ConstraintMaker(0) >= 3);
    dnfConstraints.at(2).push_back(ConstraintMaker(1) >= 1);
    dnfConstraints.at(2).push_back(ConstraintMaker(1) <= 1);
    dnfConstraints.at(3).push_back(ConstraintMaker(0) > 3);
    dnfConstraints.at(3).push_back(ConstraintMaker(1) > 1);
    dnfConstraints.at(3).push_back(ConstraintMaker(1) < 2);
    dnfConstraints.at(4).push_back(ConstraintMaker(0) >= 4);
    dnfConstraints.at(4).push_back(ConstraintMaker(1) >= 2);
    dnfConstraints.at(4).push_back(ConstraintMaker(1) <= 2);

    const auto negated = negate(dnfConstraints);
    BOOST_CHECK_EQUAL(6, negated.size());
    BOOST_CHECK_EQUAL((std::vector<Constraint>{ConstraintMaker(0) < 2}), negated.at(0));
    BOOST_CHECK_EQUAL((std::vector<Constraint>{ConstraintMaker(1) > 0, ConstraintMaker(0) <= 2}), negated.at(1));
    BOOST_CHECK_EQUAL((std::vector<Constraint>{ConstraintMaker(1) >= 1, ConstraintMaker(0) < 3}), negated.at(2));
    BOOST_CHECK_EQUAL((std::vector<Constraint>{ConstraintMaker(1) > 1, ConstraintMaker(0) <= 3}), negated.at(3));
    BOOST_CHECK_EQUAL((std::vector<Constraint>{ConstraintMaker(1) >= 2, ConstraintMaker(0) < 4}), negated.at(4));
    BOOST_CHECK_EQUAL((std::vector<Constraint>{ConstraintMaker(1) > 2}), negated.at(5));
  }

  BOOST_AUTO_TEST_CASE(negateDuplicated) {
    std::vector<std::vector<Constraint>> dnfConstraints;
    dnfConstraints.resize(3);
    dnfConstraints.at(0).push_back(ConstraintMaker(0) >= 1);
    dnfConstraints.at(1).push_back(ConstraintMaker(0) < 1);
    dnfConstraints.at(2).push_back(ConstraintMaker(0) >= 1);

    const auto negated = negate(dnfConstraints);
    BOOST_TEST(negated.empty());
  }

  BOOST_AUTO_TEST_CASE(simplifyTest) {
    std::vector<std::vector<Constraint>> inputs;
    std::vector<std::vector<Constraint>> expected;
    inputs.resize(2);
    expected.resize(2);
    inputs.at(0) = {ConstraintMaker(0) < 2, ConstraintMaker(0) <= 2};
    expected.at(0) = {ConstraintMaker(0) < 2};
    inputs.at(1) = {ConstraintMaker(1) > 0, ConstraintMaker(0) <= 2};
    expected.at(1) = {ConstraintMaker(1) > 0, ConstraintMaker(0) <= 2};
    for (int i = 0; i < inputs.size(); ++i) {
      BOOST_CHECK_EQUAL(expected.at(i), simplify(inputs.at(i)));
    }
  }

  BOOST_AUTO_TEST_CASE(satisfiableTest) {
    std::vector<std::vector<Constraint>> inputs;
    std::vector<bool> expected;
    inputs.resize(4);
    expected.resize(4);
    inputs.at(0) = {ConstraintMaker(0) < 2, ConstraintMaker(0) <= 2};
    expected.at(0) = true;
    inputs.at(1) = {ConstraintMaker(1) > 0, ConstraintMaker(0) <= 2};
    expected.at(1) = true;
    inputs.at(2) = {ConstraintMaker(0) < 2, ConstraintMaker(1) <= 2};
    expected.at(2) = true;
    inputs.at(3) = {ConstraintMaker(1) > 0, ConstraintMaker(1) <= 0};
    expected.at(3) = false;
    for (int i = 0; i < inputs.size(); ++i) {
      BOOST_CHECK_EQUAL(expected.at(i), satisfiable(inputs.at(i)));
    }
    /*
     *
     * x1 > 0 && x0 <= 2 &&
x0 < 2 && x1 <= 0 &&
x1 > 0 && x1 <= 0 &&
x0 < 2 && x1 >= 1 &&
x1 > 0 && x1 >= 1 &&
x1 < 0 && x0 < 3 &&
x0 >= 2 && x1 < 0 && x1 < 1 &&

     x0 > 2 && x1 <= 1 && x1 < 1 &&
     */
  }

  BOOST_AUTO_TEST_CASE(isWeakerTest) {
    // TODO: Implement
    // TODO: Set up rapid check
    // different order should be false
    BOOST_TEST(!(ConstraintMaker(0) < 2).isWeaker(ConstraintMaker(0) > 3));
    BOOST_TEST(!(ConstraintMaker(0) < 2).isWeaker(ConstraintMaker(0) >= 3));
    BOOST_TEST(!(ConstraintMaker(0) <= 2).isWeaker(ConstraintMaker(0) > 3));
    BOOST_TEST(!(ConstraintMaker(0) <= 2).isWeaker(ConstraintMaker(0) >= 3));
    BOOST_TEST(!(ConstraintMaker(0) > 2).isWeaker(ConstraintMaker(0) < 3));
    BOOST_TEST(!(ConstraintMaker(0) > 2).isWeaker(ConstraintMaker(0) <= 3));
    BOOST_TEST(!(ConstraintMaker(0) >= 2).isWeaker(ConstraintMaker(0) <= 3));
    BOOST_TEST(!(ConstraintMaker(0) >= 2).isWeaker(ConstraintMaker(0) <= 3));
    // different variable should be false
    BOOST_TEST(!(ConstraintMaker(0) < 2).isWeaker(ConstraintMaker(1) < 3));
    BOOST_TEST(!(ConstraintMaker(0) >= 2).isWeaker(ConstraintMaker(1) >= 3));
    BOOST_TEST(!(ConstraintMaker(1) <= 2).isWeaker(ConstraintMaker(0) <= 3));
    BOOST_TEST(!(ConstraintMaker(1) > 2).isWeaker(ConstraintMaker(0) > 3));
    // Consistent
    BOOST_TEST(!(ConstraintMaker(0) < 2).isWeaker(ConstraintMaker(0) < 3));
    BOOST_TEST((ConstraintMaker(0) >= 2).isWeaker(ConstraintMaker(0) >= 3));
    BOOST_TEST(!(ConstraintMaker(0) <= 2).isWeaker(ConstraintMaker(0) <= 3));
    BOOST_TEST((ConstraintMaker(0) > 2).isWeaker(ConstraintMaker(0) > 3));
    BOOST_TEST((ConstraintMaker(0) < 3).isWeaker(ConstraintMaker(0) < 2));
    BOOST_TEST(!(ConstraintMaker(0) >= 3).isWeaker(ConstraintMaker(0) >= 2));
    BOOST_TEST((ConstraintMaker(0) <= 3).isWeaker(ConstraintMaker(0) <= 2));
    BOOST_TEST(!(ConstraintMaker(0) > 3).isWeaker(ConstraintMaker(0) > 2));
    // Same except for the equality
    BOOST_TEST(!(ConstraintMaker(0) < 2).isWeaker(ConstraintMaker(0) <= 2));
    BOOST_TEST((ConstraintMaker(0) <= 2).isWeaker(ConstraintMaker(0) < 2));
  }

  BOOST_AUTO_TEST_CASE(unionHullTest) {
    std::vector<std::vector<Constraint>> guards = {
            {ConstraintMaker(0) > 2, ConstraintMaker(0) < 3, ConstraintMaker(1) >= 1, ConstraintMaker(1) <= 1},
            {ConstraintMaker(0) >= 2, ConstraintMaker(0) <= 2, ConstraintMaker(1) > 0, ConstraintMaker(1) < 1},
    };
    std::vector<Constraint> expected = {ConstraintMaker(0) >= 2, ConstraintMaker(0) < 3, ConstraintMaker(1) > 0, ConstraintMaker(1) <= 1};
    BOOST_TEST(isWeaker(expected, unionHull(guards)));
    BOOST_TEST(isWeaker(unionHull(guards), expected));
  }
BOOST_AUTO_TEST_SUITE_END()
/**
 * @author Masaki Waga
 * @date 2022/03/05.
 */
#include <boost/test/unit_test.hpp>

#define private public

#include "fractional_order.hh"

BOOST_AUTO_TEST_SUITE(FractionalOrderTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(successorEq) {
    FractionalOrder order;
    order.order = {{1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.successor();
    BOOST_REQUIRE_EQUAL(4, order.order.size());
    auto it = order.order.begin();
    BOOST_TEST(it->empty());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    BOOST_CHECK_EQUAL(2, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
  }

  BOOST_AUTO_TEST_CASE(successorNeq) {
    FractionalOrder order;
    order.order = {{},
                   {1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.successor();
    BOOST_REQUIRE_EQUAL(3, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    BOOST_CHECK_EQUAL(2, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
  }

  BOOST_AUTO_TEST_CASE(predecessorEq) {
    FractionalOrder order;
    order.order = {{1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.predecessor();
    BOOST_REQUIRE_EQUAL(4, order.order.size());
    auto it = order.order.begin();
    BOOST_TEST(it->empty());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    BOOST_CHECK_EQUAL(2, it->back());
  }

  BOOST_AUTO_TEST_CASE(predecessorNeq) {
    FractionalOrder order;
    order.order = {{},
                   {1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.predecessor();
    BOOST_REQUIRE_EQUAL(3, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    BOOST_CHECK_EQUAL(2, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
  }

  BOOST_AUTO_TEST_CASE(extendEqEq) {
    FractionalOrder order;
    order.order = {{1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.extendEq();
    BOOST_REQUIRE_EQUAL(3, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    BOOST_CHECK_EQUAL(2, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
    BOOST_CHECK_EQUAL(4, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
  }

  BOOST_AUTO_TEST_CASE(extendEqNeq) {
    FractionalOrder order;
    order.order = {{},
                   {1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.extendEq();
    BOOST_REQUIRE_EQUAL(4, order.order.size());
    auto it = order.order.begin();
    BOOST_TEST(it->empty());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(1, it->front());
    BOOST_CHECK_EQUAL(2, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
    BOOST_CHECK_EQUAL(4, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
  }

  BOOST_AUTO_TEST_CASE(extendZeroEq) {
    FractionalOrder order;
    order.order = {{1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.extendZero();
    BOOST_REQUIRE_EQUAL(3, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(2, it->front());
    BOOST_CHECK_EQUAL(3, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(4, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    BOOST_CHECK_EQUAL(1, it->back());
  }

  BOOST_AUTO_TEST_CASE(extendZeroNeq) {
    FractionalOrder order;
    order.order = {{},
                   {1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order.extendZero();
    BOOST_REQUIRE_EQUAL(4, order.order.size());
    auto it = order.order.begin();
    BOOST_TEST(it->empty());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(2, it->front());
    BOOST_CHECK_EQUAL(3, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(4, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    BOOST_CHECK_EQUAL(1, it->back());
  }

BOOST_AUTO_TEST_SUITE_END()

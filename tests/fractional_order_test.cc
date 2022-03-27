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
    order = order.successor();
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
    order = order.successor();
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
    order = order.predecessor();
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
    order = order.predecessor();
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

  BOOST_AUTO_TEST_CASE(extendNEq) {
    FractionalOrder order;
    order.order = {{1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order = order.extendN();
    BOOST_REQUIRE_EQUAL(3, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(3, it->size());
    auto ij = it->begin();
    BOOST_CHECK_EQUAL(1, *ij++);
    BOOST_CHECK_EQUAL(2, *ij++);
    BOOST_CHECK_EQUAL(4, *ij++);
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(3, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
  }

  BOOST_AUTO_TEST_CASE(extendNNeq) {
    FractionalOrder order;
    order.order = {{},
                   {1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order = order.extendN();
    BOOST_REQUIRE_EQUAL(4, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(4, it->front());
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

  BOOST_AUTO_TEST_CASE(extendZeroEq) {
    FractionalOrder order;
    order.order = {{1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order = order.extendZero();
    BOOST_REQUIRE_EQUAL(3, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(3, it->size());
    auto ij = it->begin();
    BOOST_CHECK_EQUAL(0, *ij++);
    BOOST_CHECK_EQUAL(2, *ij++);
    BOOST_CHECK_EQUAL(3, *ij++);
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(4, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->back());
  }

  BOOST_AUTO_TEST_CASE(extendZeroNeq) {
    FractionalOrder order;
    order.order = {{},
                   {1, 2},
                   {3},
                   {0}};
    order.size = 4;
    order = order.extendZero();
    BOOST_REQUIRE_EQUAL(4, order.order.size());
    auto it = order.order.begin();
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(0, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(2, it->size());
    BOOST_CHECK_EQUAL(2, it->front());
    BOOST_CHECK_EQUAL(3, it->back());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(4, it->front());
    it++;
    BOOST_REQUIRE_EQUAL(1, it->size());
    BOOST_CHECK_EQUAL(1, it->back());
  }

  BOOST_AUTO_TEST_CASE(fromFractionalParts) {
    std::vector<double> fractionalParts = {0.5, 0.5, 0.5, 0};
    const auto fractionalOrder = FractionalOrder{fractionalParts};

    BOOST_CHECK_EQUAL(4, fractionalOrder.size);
    BOOST_CHECK_EQUAL(2, fractionalOrder.order.size());
    BOOST_CHECK_EQUAL(1, fractionalOrder.order.front().size());
    BOOST_CHECK_EQUAL(3, fractionalOrder.order.front().front());
    BOOST_CHECK_EQUAL(3, fractionalOrder.order.back().size());
    BOOST_CHECK_EQUAL(0, fractionalOrder.order.back().front());
    BOOST_CHECK_EQUAL(1, *std::next(fractionalOrder.order.back().begin()));
    BOOST_CHECK_EQUAL(2, fractionalOrder.order.back().back());
  }

BOOST_AUTO_TEST_SUITE_END()

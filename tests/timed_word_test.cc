/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../include/timed_word.hh"

BOOST_AUTO_TEST_SUITE(TimedWordTest)

  using namespace learnta;
  BOOST_AUTO_TEST_CASE(print) {
    std::stringstream stream;
    TimedWord word {"ab", {0.8, 1.2, 3.0}};
    word.print(stream);
    BOOST_CHECK_EQUAL("0.8 a 1.2 b 3", stream.str());
  }
BOOST_AUTO_TEST_SUITE_END()

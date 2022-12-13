/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../include/timed_word.hh"

BOOST_AUTO_TEST_SUITE(TimedWordTest)

  using namespace learnta;

  BOOST_AUTO_TEST_CASE(getSuffix) {
    TimedWord word{"ab", {0.8, 1.2, 3.0}};
    TimedWord prefix{"a", {0.8, 0.4}};
    TimedWord suffix{"b", {1.2 - 0.4, 3.0}};
    BOOST_CHECK_EQUAL(suffix, word.getSuffix(prefix));
  }

  BOOST_AUTO_TEST_CASE(getSuffixEndWithZero) {
    TimedWord word{"ab", {0.8, 1.2, 3.0}};
    TimedWord prefix{"a", {0.8, 0.0}};
    TimedWord suffix{"b", {1.2, 3.0}};
    BOOST_CHECK_EQUAL(suffix, word.getSuffix(prefix));
  }

  BOOST_AUTO_TEST_CASE(getSuffixBeginWithZero) {
    TimedWord word{"ab", {0.8, 1.2, 3.0}};
    TimedWord prefix{"a", {0.8, 1.2}};
    TimedWord suffix{"b", {0.0, 3.0}};
    BOOST_CHECK_EQUAL(suffix, word.getSuffix(prefix));
  }

  BOOST_AUTO_TEST_CASE(print) {
    std::stringstream stream;
    TimedWord word{"ab", {0.8, 1.2, 3.0}};
    word.print(stream);
    BOOST_CHECK_EQUAL("0.8 a 1.2 b 3", stream.str());
  }

  BOOST_AUTO_TEST_CASE(normalize) {
    std::string w = "a";
    w.push_back(UNOBSERVABLE);
    w.push_back('c');
    TimedWord word{w, {0.8, 1.2, 3.0, 2.0}};
    BOOST_CHECK_EQUAL("ac", word.getWord());
    const std::vector<double> expectedDurations {0.8, 4.2, 2.0};
    BOOST_CHECK_EQUAL_COLLECTIONS(expectedDurations.begin(), expectedDurations.end(),
                                  word.getDurations().begin(), word.getDurations().end());
  }
BOOST_AUTO_TEST_SUITE_END()

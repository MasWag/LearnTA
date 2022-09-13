/**
 * @author Masaki Waga
 * @date 2022/09/13.
 */

#include <boost/test/unit_test.hpp>
#include <sstream>

#include "../include/timed_automaton.hh"

#include "simple_automaton_fixture.hh"

BOOST_AUTO_TEST_SUITE(TimedAutomatonTest)

  using namespace learnta;

  BOOST_FIXTURE_TEST_CASE(print, SimpleAutomatonFixture) {
    std::stringstream stream;
    stream << this->automaton;
    std::string expected = "digraph G {\n";
    expected += "        loc0 [init=1, match=1]\n";
    expected += "        loc1 [init=0, match=0]\n";
    expected += "        loc0->loc0 [label=\"a\", guard=\"{x0 < 1}\"]\n";
    expected += "        loc0->loc1 [label=\"a\", guard=\"{x0 >= 1}\", reset=\"{x0 := 0}\"]\n";
    expected += "        loc1->loc0 [label=\"a\", guard=\"{x0 <= 1}\"]\n";
    expected += "        loc1->loc1 [label=\"a\", guard=\"{x0 > 1}\"]\n";
    expected += "}\n";
    BOOST_CHECK_EQUAL(expected, stream.str());
  }

BOOST_AUTO_TEST_SUITE_END()

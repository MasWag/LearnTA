/**
 * @author Masaki Waga
 * @date 2022/09/13.
 */

#include <boost/test/unit_test.hpp>
#include <sstream>

#include "../include/timed_automaton.hh"

#include "simple_automaton_fixture.hh"
#include "unbalanced_fixture.hh"

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

  BOOST_AUTO_TEST_CASE(printUnobservableTransitions) {
    learnta::TimedAutomaton automaton;
    automaton.states.resize(1);
    automaton.states.at(0) = std::make_shared<learnta::TAState>(true);
    automaton.states.at(0)->next[0].resize(1);
    // Transitions from loc0
    automaton.states.at(0)->next[0].at(0).target = automaton.states.at(0).get();
    automaton.states.at(0)->next[0].at(0).guard = {learnta::ConstraintMaker(0) < 1};

    automaton.initialStates.push_back(automaton.states.at(0));
    automaton.maxConstraints.resize(1);
    automaton.maxConstraints[0] = 1;

    std::stringstream stream;
    stream << automaton;
    std::string expected = "digraph G {\n";
    expected += "        loc0 [init=1, match=1]\n";
    expected += "        loc0->loc0 [label=\"ε\", guard=\"{x0 < 1}\"]\n";
    expected += "}\n";
    BOOST_CHECK_EQUAL(expected, stream.str());
  }

  BOOST_FIXTURE_TEST_CASE(makeCompleteUnobservable, SimpleAutomatonWithOneUnobservableFixture) {
    TimedAutomaton original;
    std::unordered_map<TAState *, std::shared_ptr<TAState>> old2new;
    this->automatonWithOneUnobservable.deepCopy(original, old2new);
    this->automatonWithOneUnobservable.makeComplete(this->alphabet);
    BOOST_CHECK_EQUAL(original.stateSize() + 1, this->automatonWithOneUnobservable.stateSize());
    BOOST_CHECK_EQUAL(original.initialStates.size(), this->automatonWithOneUnobservable.initialStates.size());
    BOOST_CHECK_EQUAL_COLLECTIONS(original.maxConstraints.begin(),
                                  original.maxConstraints.end(),
                                  this->automatonWithOneUnobservable.maxConstraints.begin(),
                                  this->automatonWithOneUnobservable.maxConstraints.end());
    this->automatonWithOneUnobservable = this->automatonWithOneUnobservable.simplify();
    BOOST_CHECK_EQUAL(original.stateSize(), this->automatonWithOneUnobservable.stateSize());
  }

  BOOST_FIXTURE_TEST_CASE(makeCompleteUnbalancedHypothesis20221219, UnbalancedHypothesis20221219Fixture) {
    TimedAutomaton original;
    std::unordered_map<TAState *, std::shared_ptr<TAState>> old2new;
    this->hypothesis.deepCopy(original, old2new);
    const std::vector<learnta::Alphabet> alphabet = {'a', 'b', 'c'};
    this->hypothesis.makeComplete(alphabet);
    BOOST_CHECK_EQUAL(original.stateSize() + 1, this->hypothesis.stateSize());
    BOOST_CHECK_EQUAL(original.initialStates.size(), this->hypothesis.initialStates.size());
  }
BOOST_AUTO_TEST_SUITE_END()

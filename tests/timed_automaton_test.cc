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

  BOOST_AUTO_TEST_CASE(mergeNondeterministicBranchingTest) {
    std::stringstream stream;
    std::vector<std::shared_ptr<TAState>> states;
    for (int i = 0; i < 3; ++i) {
      states.push_back(std::make_shared<TAState>(i % 2));
    }
    std::array<TATransition::Resets, 2> resets;
    resets.at(0).emplace_back(0, 6.0);
    resets.at(0).emplace_back(2, 0.0);
    resets.at(1).emplace_back(2, 0.0);
    resets.at(1).emplace_back(0, 5.5);

    states.at(0)->next['b'].emplace_back(states.at(1).get(), resets.at(0),
                                         std::vector<Constraint>{ConstraintMaker(1) > 5, ConstraintMaker(0) < 8,
                                                                  ConstraintMaker(0) > 6, ConstraintMaker(1) < 6});
    states.at(0)->next['b'].emplace_back(states.at(2).get(), resets.at(1),
                                         std::vector<Constraint>{ConstraintMaker(1) > 5, ConstraintMaker(0) < 7,
                                                                 ConstraintMaker(0) > 5, ConstraintMaker(1) < 6});

    std::cout << TimedAutomaton{{states, {states.front()}}, TimedAutomaton::makeMaxConstants(states)};
    stream << TimedAutomaton{{states, {states.front()}}, TimedAutomaton::makeMaxConstants(states)};
    BOOST_CHECK_EQUAL("digraph G {\n"
                      "        loc0 [init=1, match=0]\n"
                      "        loc1 [init=0, match=1]\n"
                      "        loc2 [init=0, match=0]\n"
                      "        loc0->loc1 [label=\"b\", guard=\"{x1 > 5, x0 < 8, x0 > 6, x1 < 6}\", reset=\"{x0 := 6, x2 := 0}\"]\n"
                      "        loc0->loc2 [label=\"b\", guard=\"{x1 > 5, x0 < 7, x0 > 5, x1 < 6}\", reset=\"{x2 := 0, x0 := 5.5}\"]\n"
                      "}\n", stream.str());
    stream.str("");
    const std::unordered_set<ClockVariables> preciseClocks = {1};
    states.at(0)->mergeNondeterministicBranching(preciseClocks);
    BOOST_CHECK_EQUAL(1, states.at(0)->next.at('b').size());
    BOOST_CHECK_EQUAL(states.at(1).get(), states.at(0)->next.at('b').front().target);
    BOOST_CHECK_EQUAL(resets.at(0), states.at(0)->next.at('b').front().resetVars);
    const auto sort = [] (std::vector<Constraint> guard) -> std::vector<Constraint> {
      std::sort(guard.begin(), guard.end(), [] (const Constraint &left, const Constraint &right) -> bool {
        return std::make_pair(left.x, left.toDBMBound()) <= std::make_pair(right.x, right.toDBMBound());
      });
      return guard;
    };
    std::vector<Constraint> expectedGuards{ConstraintMaker(1) > 5, ConstraintMaker(0) < 8,
                                           ConstraintMaker(0) > 5, ConstraintMaker(1) < 6};
    BOOST_CHECK_EQUAL(sort(expectedGuards), sort(states.at(0)->next.at('b').front().guard));
  }
BOOST_AUTO_TEST_SUITE_END()

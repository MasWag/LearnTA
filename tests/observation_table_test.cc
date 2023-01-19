/**
 * @author Masaki Waga
 * @date 2022/03/22.
 */

#include <iostream>
#include <boost/test/unit_test.hpp>

#define private public
#define protected public

#include "../include/timed_automaton_runner.hh"
#include "../include/symbolic_membership_oracle.hh"
#include "../include/equivalence_oracle.hh"
#include "../include/timed_automata_equivalence_oracle.hh"

#include "simple_automaton_fixture.hh"
#include "simple_observation_table_keys_fixture.hh"
#include "observation_table.hh"

using namespace learnta;

BOOST_AUTO_TEST_SUITE(ObservationTableTest)

  struct SimpleAutomatonOracleFixture : public SimpleAutomatonFixture, ComplementSimpleAutomatonFixture, SimpleObservationTableKeysFixture {
    std::unique_ptr<learnta::SymbolicMembershipOracle> memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(
            std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner{this->automaton}));
    const std::vector<Alphabet> alphabet = {'a'};
    ObservationTable observationTable;

    SimpleAutomatonOracleFixture() :
            observationTable(alphabet,
                             std::make_unique<learnta::SymbolicMembershipOracle>(
                                     std::unique_ptr<learnta::SUL>(
                                             new learnta::TimedAutomatonRunner{this->automaton}))) {}
  };


  BOOST_FIXTURE_TEST_CASE(initialHypothesis, SimpleAutomatonOracleFixture) {
    BOOST_CHECK(observationTable.close());
    BOOST_CHECK(observationTable.consistent());
    BOOST_CHECK(observationTable.exteriorConsistent());

    BOOST_CHECK_EQUAL(3, this->observationTable.prefixes.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.pIndices.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.suffixes.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.discreteSuccessors.size());
    BOOST_CHECK_EQUAL(1, this->observationTable.discreteSuccessors.at(std::make_pair(0, 'a')));
    BOOST_CHECK_EQUAL(1, this->observationTable.continuousSuccessors.size());
    BOOST_CHECK_EQUAL(2, this->observationTable.continuousSuccessors.at(0));

    // All the cells should be "top"
    for (const auto &prefix: this->observationTable.prefixes) {
      const auto concat = prefix + this->observationTable.suffixes.at(0);
      auto result = this->memOracle->query(concat);
      BOOST_CHECK_EQUAL(1, result.size());
      BOOST_CHECK_EQUAL(concat.getTimedCondition(), result.front());
    }

    const auto hypothesis = this->observationTable.generateHypothesis();

    BOOST_CHECK_EQUAL(1, hypothesis.stateSize());
    BOOST_CHECK_EQUAL(1, hypothesis.initialStates.size());
    BOOST_CHECK(hypothesis.states.front()->isMatch);
    /*
    BOOST_CHECK_EQUAL(1, hypothesis.states.front()->next.at('a').size());
    BOOST_CHECK(hypothesis.states.front()->next.at('a').front().resetVars.empty());
    BOOST_CHECK(hypothesis.states.front()->next.at('a').front().guard.empty());
    BOOST_CHECK(hypothesis.maxConstraints.empty());
     */
  }

  BOOST_AUTO_TEST_CASE(stateSplitTest) {
    const auto toTA = [] (std::vector<std::shared_ptr<TAState>> states) {
      return TimedAutomaton{{states, {states.front()}}, TimedAutomaton::makeMaxConstants(states)}.simplify();
    };
    const auto sort = [] (std::vector<Constraint> guard) -> std::vector<Constraint> {
      std::sort(guard.begin(), guard.end(), [] (const Constraint &left, const Constraint &right) -> bool {
        return std::make_pair(left.x, left.toDBMBound()) <= std::make_pair(right.x, right.toDBMBound());
      });
      return guard;
    };
    std::stringstream stream;
    std::vector<std::shared_ptr<TAState>> states;
    states.reserve(3);
    states.push_back(std::make_shared<TAState>(false));
    states.push_back(std::make_shared<TAState>(false));
    states.push_back(std::make_shared<TAState>(true));
    states.at(0)->next['a'].emplace_back(states.at(1).get(), TATransition::Resets{{1, 0.0}},
                                         std::vector<Constraint>{ConstraintMaker(0) < 2, ConstraintMaker(0) > 1});
    states.at(1)->next['a'].emplace_back(states.at(1).get(), TATransition::Resets{{1, 0.5}},
                                         std::vector<Constraint>{ConstraintMaker(0) < 2, ConstraintMaker(0) > 1,
                                                                 ConstraintMaker(1) < 1, ConstraintMaker(1) > 0});
    states.at(1)->next['a'].emplace_back(states.at(1).get(), TATransition::Resets{{0, 1.5}, {1, 0.0}},
                                         std::vector<Constraint>{ConstraintMaker(0) < 3, ConstraintMaker(0) > 2,
                                                                 ConstraintMaker(1) <= 1, ConstraintMaker(1) >= 1});
    states.at(1)->next['b'].emplace_back(states.at(0).get(), TATransition::Resets{},
                                         std::vector<Constraint>{ConstraintMaker(0) < 2, ConstraintMaker(0) > 1,
                                                                 ConstraintMaker(1) <= 1, ConstraintMaker(1) >= 1});
    states.at(1)->next['b'].emplace_back(states.at(2).get(), TATransition::Resets{{2, 0.0}},
                                         std::vector<Constraint>{ConstraintMaker(0) < 2, ConstraintMaker(0) > 1,
                                                                 ConstraintMaker(1) < 1, ConstraintMaker(1) > 0});
    states.at(2)->next['b'].emplace_back(states.at(0).get(), TATransition::Resets{},
                                         std::vector<Constraint>{ConstraintMaker(0) < 2, ConstraintMaker(0) > 1,
                                                                 ConstraintMaker(1) < 1, ConstraintMaker(1) > 0,
                                                                 ConstraintMaker(2) < 1, ConstraintMaker(2) > 0});
    std::cout << toTA(states) << std::endl;
    ImpreciseClockHandler impreciseNeighbors;
    std::array<RenamingRelation, 2> renamings;
    renamings.at(0).emplace_back(0, 0);
    renamings.at(1).emplace_back(1, 1);
    impreciseNeighbors.push(states.at(1).get(), renamings.at(0),
                            ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"a", {1.1, 0.1}}));
    impreciseNeighbors.push(states.at(1).get(), renamings.at(1),
                            ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"a", {1.1, 1.0}}));
    impreciseNeighbors.run();
    std::cout << toTA(states) << std::endl;
    std::vector<TAState *> needSplit;
    needSplit.reserve(states.size());
    for (const auto &state: states) {
      if (state->needSplitting()) {
        needSplit.push_back(state.get());
      }
    }
    BOOST_CHECK_EQUAL(1, needSplit.size());
    BOOST_CHECK_EQUAL(states.at(1).get(), needSplit.front());
    stream << toTA(states);
    BOOST_CHECK_EQUAL("digraph G {\n"
                      "        loc0 [init=1, match=0]\n"
                      "        loc1 [init=0, match=0]\n"
                      "        loc2 [init=0, match=1]\n"
                      "        loc0->loc1 [label=\"a\", guard=\"{x0 < 2, x0 > 1}\", reset=\"{x1 := 0}\"]\n"
                      "        loc1->loc0 [label=\"b\", guard=\"{x0 < 2, x0 > 1, x1 <= 1, x1 >= 1}\"]\n"
                      "        loc1->loc2 [label=\"b\", guard=\"{x0 < 2, x0 > 1, x1 < 1, x1 > 0}\", reset=\"{x2 := 0}\"]\n"
                      "        loc1->loc2 [label=\"b\", guard=\"{x1 > 0, x0 < 2, x0 > 1, x1 < 2}\", reset=\"{x2 := 0, x1 := 0.25}\"]\n"
                      "        loc1->loc2 [label=\"b\", guard=\"{x1 > 0, x0 < 2, x0 > 1, x1 < 2}\", reset=\"{x2 := 0, x1 := 0.5}\"]\n"
                      "        loc1->loc1 [label=\"a\", guard=\"{x0 < 2, x0 > 1, x1 < 1, x1 > 0}\", reset=\"{x1 := 0.5}\"]\n"
                      "        loc1->loc1 [label=\"a\", guard=\"{x0 < 3, x0 > 2, x1 <= 1, x1 >= 1}\", reset=\"{x0 := 1.5, x1 := 0}\"]\n"
                      "        loc1->loc1 [label=\"a\", guard=\"{x1 > 0, x0 < 2, x0 > 1, x1 < 2}\", reset=\"{x1 := 0.5}\"]\n"
                      "        loc2->loc0 [label=\"b\", guard=\"{x0 < 2, x0 > 1, x1 < 1, x1 > 0, x2 < 1, x2 > 0}\"]\n"
                      "        loc2->loc0 [label=\"b\", guard=\"{x2 > 0, x1 > 0, x0 < 2, x2 < 1, x0 > 1, x1 < 2}\"]\n"
                      "}\n", stream.str());
    auto initialState = states.at(0);
    const auto originalInitialState = initialState;
    ObservationTable::splitStates(states, initialState, needSplit);
    BOOST_TEST(originalInitialState != initialState);
    BOOST_TEST(states.front() == initialState);
    for (const auto& state: states) {
      for (const auto& [action, transitions]: state->next) {
        for (const auto& transition: transitions) {
          auto it = std::find_if(states.begin(), states.end(), [&] (const auto& state) {
            return state.get() == transition.target;
          });
          BOOST_TEST((it != states.end()));
        }
      }
    }
    // Test the transitions from states.at(0)
    BOOST_CHECK_EQUAL(1, states.at(0)->next.at('a').size());
    BOOST_CHECK_EQUAL(states.at(1).get(), states.at(0)->next.at('a').at(0).target);
    BOOST_CHECK_EQUAL(sort(std::vector<Constraint>{ConstraintMaker(0) < 2, ConstraintMaker(0) > 1}),
                      sort(states.at(0)->next.at('a').at(0).guard));
    TATransition::Resets expectedReset;
    expectedReset.emplace_back(1, 0.0);
    BOOST_CHECK_EQUAL(expectedReset, states.at(0)->next.at('a').at(0).resetVars);
    // Test the transitions from states.at(1)
    BOOST_CHECK_EQUAL(2, states.at(1)->next.at('a').size());
    BOOST_CHECK_EQUAL(2, states.at(1)->next.at('b').size());
    for (const auto& [action, transitions]: states.at(1)->next) {
      for (const auto& transition: transitions) {
        BOOST_TEST((std::vector<ClockVariables>{0, 1}) == simpleVariables(transition.guard),
                   boost::test_tools::per_element());
      }
    }
    BOOST_TEST(std::any_of(states.at(1)->next.at('b').begin(), states.at(1)->next.at('b').end(),
                           [&] (const TATransition &transition) {
      return transition.target == states.at(0).get();
    }));
    BOOST_TEST(std::any_of(states.at(1)->next.at('b').begin(), states.at(1)->next.at('b').end(),
                           [&] (const TATransition &transition) {
      return transition.target == states.at(2).get();
    }));
    // Test the transitions from states.at(2)
    BOOST_CHECK_EQUAL(1, states.at(2)->next.at('b').size());
    BOOST_CHECK_EQUAL(states.at(0).get(), states.at(2)->next.at('b').at(0).target);
    BOOST_CHECK_EQUAL(sort(std::vector<Constraint>{ConstraintMaker(0) < 2, ConstraintMaker(0) > 1,
                                                   ConstraintMaker(1) < 1, ConstraintMaker(1) > 0,
                                                   ConstraintMaker(2) < 1, ConstraintMaker(2) > 0}),
                      sort(states.at(2)->next.at('b').at(0).guard));
    for (const auto& [action, transitions]: states.at(2)->next) {
      for (const auto& transition: transitions) {
        BOOST_TEST((std::vector<ClockVariables>{0, 1, 2}) == simpleVariables(transition.guard),
                   boost::test_tools::per_element());
      }
    }
    std::cout << toTA(states) << std::endl;
  }
BOOST_AUTO_TEST_SUITE_END()

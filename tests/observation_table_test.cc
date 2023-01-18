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
    std::cout << toTA(states) << std::endl;
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
    std::cout << toTA(states) << std::endl;
  }
BOOST_AUTO_TEST_SUITE_END()

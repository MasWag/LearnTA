/**
 * @author Masaki Waga
 * @date 2022/03/16.
 */

#include <boost/test/unit_test.hpp>

#define private public

#include "../include/timed_automata_equivalence_oracle.hh"
#include "simple_automaton_fixture.hh"
#include "light_automaton_fixture.hh"

#include "manual_eq_tester.hh"
#include "unbalanced_fixture.hh"
#include "../examples/unbalanced_fixture.hh"

BOOST_AUTO_TEST_SUITE(TimedAutomataEquivalenceOracleTest)

  using namespace learnta;
  struct Fixture
          : public SimpleAutomatonFixture, public UniversalAutomatonFixture, public ComplementSimpleAutomatonFixture {
  };

  struct FixtureWithUnobservable : public Fixture, public SimpleAutomatonWithOneUnobservableFixture, public SimpleAutomatonWithTwoUnobservableFixture {};

  BOOST_FIXTURE_TEST_CASE(query, Fixture) {
    auto oracle = ComplementTimedAutomataEquivalenceOracle{this->automaton, this->complementAutomaton, {'a'}};

    BOOST_CHECK(oracle.findCounterExample(this->universalAutomaton));
    auto counterexample = oracle.findCounterExample(this->universalAutomaton).value();
    BOOST_CHECK_EQUAL(1, counterexample.wordSize());
    auto complementRunner = TimedAutomatonRunner{this->complementAutomaton};
    auto universalRunner = TimedAutomatonRunner{this->universalAutomaton};
    BOOST_CHECK(!complementRunner.step(counterexample.getDurations().at(0)));
    BOOST_CHECK(universalRunner.step(counterexample.getDurations().at(0)));
    BOOST_CHECK(complementRunner.step(counterexample.getWord().at(0)));
    BOOST_CHECK(universalRunner.step(counterexample.getWord().at(0)));
    BOOST_CHECK(complementRunner.step(counterexample.getDurations().at(1)));
    BOOST_CHECK(universalRunner.step(counterexample.getDurations().at(1)));

    BOOST_CHECK(!oracle.findCounterExample(this->automaton));
  }

  BOOST_AUTO_TEST_CASE(light19) {
    auto fixture = LightAutomatonFixture(19);
    auto oracle = ComplementTimedAutomataEquivalenceOracle{fixture.targetAutomaton,
                                                           fixture.complementTargetAutomaton,
                                                           fixture.alphabet};

    TimedAutomaton wrongAutomaton;
    wrongAutomaton.states.reserve(5);
    for (int i = 0; i < 4; ++i) {
      wrongAutomaton.states.push_back(std::make_shared<TAState>(true));
    }
    // the sink state
    wrongAutomaton.states.push_back(std::make_shared<TAState>(false));
    wrongAutomaton.initialStates.push_back(wrongAutomaton.states.front());
    wrongAutomaton.maxConstraints = {38, 38, 0};

    // loc0->loc1 [label="p", reset="{x1 := 0}"]
    wrongAutomaton.states.at(0)->next['p'].emplace_back();
    wrongAutomaton.states.at(0)->next['p'].back().target = wrongAutomaton.states.at(1).get();
    wrongAutomaton.states.at(0)->next['p'].back().resetVars = {{1, 0.0}};

    // loc1->loc3 [label="s", guard="{x0 >= 38, x1 >= 38}", reset="{x2 := 0}"]
    wrongAutomaton.states.at(1)->next['s'].emplace_back();
    wrongAutomaton.states.at(1)->next['s'].back().target = wrongAutomaton.states.at(3).get();
    wrongAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(0) >= 38,
                                                           learnta::ConstraintMaker(1) >= 38};
    wrongAutomaton.states.at(1)->next['s'].back().resetVars = {{2, 0.0}};

    // Transition to the sink state for the totality
    wrongAutomaton.states.at(1)->next['s'].emplace_back();
    wrongAutomaton.states.at(1)->next['s'].back().target = wrongAutomaton.states.back().get();
    wrongAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(1) < 38};

    // loc1->loc2 [label="r", guard="{x0 >= 19, x1 >= 19, x1 <= 19}", reset="{x2 := 0}"]
    wrongAutomaton.states.at(1)->next['r'].emplace_back();
    wrongAutomaton.states.at(1)->next['r'].back().target = wrongAutomaton.states.at(2).get();
    wrongAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= 19,
                                                           learnta::ConstraintMaker(1) >= 19,
                                                           learnta::ConstraintMaker(1) <= 19};
    wrongAutomaton.states.at(1)->next['r'].back().resetVars = {{2, 0.0}};

    // loc1->loc2 [label="r", guard="{x0 > 19, x1 > 19, x1 < 20}"]
    wrongAutomaton.states.at(1)->next['r'].emplace_back();
    wrongAutomaton.states.at(1)->next['r'].back().target = wrongAutomaton.states.at(2).get();
    wrongAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) > 19,
                                                           learnta::ConstraintMaker(1) > 19,
                                                           learnta::ConstraintMaker(1) < 20};

    // loc1->loc2 [label="r", guard="{x0 >= 20, x1 >= 20, x1 < 38}"]
    wrongAutomaton.states.at(1)->next['r'].emplace_back();
    wrongAutomaton.states.at(1)->next['r'].back().target = wrongAutomaton.states.at(2).get();
    wrongAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= 20,
                                                           learnta::ConstraintMaker(1) >= 20,
                                                           learnta::ConstraintMaker(1) < 38};

    // loc1->loc2 [label="r", guard="{x0 >= 38, x1 >= 38}"]
    wrongAutomaton.states.at(1)->next['r'].emplace_back();
    wrongAutomaton.states.at(1)->next['r'].back().target = wrongAutomaton.states.at(2).get();
    wrongAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= 38,
                                                           learnta::ConstraintMaker(1) >= 38};

    // loc1->loc0 [label="r", guard="{x1 <= 0}"]
    wrongAutomaton.states.at(1)->next['r'].emplace_back();
    wrongAutomaton.states.at(1)->next['r'].back().target = wrongAutomaton.states.at(0).get();
    wrongAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(1) <= 0};

    // loc1->loc0 [label="r", guard="{x0 > 0, x1 > 0, x1 < 1}"]
    wrongAutomaton.states.at(1)->next['r'].emplace_back();
    wrongAutomaton.states.at(1)->next['r'].back().target = wrongAutomaton.states.at(0).get();
    wrongAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) > 0,
                                                           learnta::ConstraintMaker(1) > 0,
                                                           learnta::ConstraintMaker(1) < 1};

    // loc1->loc0 [label="r", guard="{x0 >= 1, x1 >= 1, x1 < 19}"]
    wrongAutomaton.states.at(1)->next['r'].emplace_back();
    wrongAutomaton.states.at(1)->next['r'].back().target = wrongAutomaton.states.at(0).get();
    wrongAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= 1,
                                                           learnta::ConstraintMaker(1) >= 1,
                                                           learnta::ConstraintMaker(1) < 19};

    // loc2->loc0 [label="t", guard="{x0 >= 19, x1 >= 19}", reset="{x0 := x2}"]
    wrongAutomaton.states.at(2)->next['t'].emplace_back();
    wrongAutomaton.states.at(2)->next['t'].back().target = wrongAutomaton.states.at(0).get();
    wrongAutomaton.states.at(2)->next['t'].back().guard = {learnta::ConstraintMaker(0) >= 19,
                                                           learnta::ConstraintMaker(1) >= 19};
    wrongAutomaton.states.at(2)->next['t'].back().resetVars = {{0, static_cast<ClockVariables>(2)}};

    // loc3->loc2 [label="r", guard="{x0 >= 38, x1 >= 38}"]
    wrongAutomaton.states.at(3)->next['r'].emplace_back();
    wrongAutomaton.states.at(3)->next['r'].back().target = wrongAutomaton.states.at(2).get();
    wrongAutomaton.states.at(3)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= 38,
                                                           learnta::ConstraintMaker(1) >= 38};

    // Manually test the equivalence
    auto correctRunner = TimedAutomatonRunner{fixture.targetAutomaton};
    auto runner = TimedAutomatonRunner{wrongAutomaton};
    ManualEqTester tester{correctRunner, runner};
    tester.run("prprp", {2, fixture.scale * 0.5, 2, fixture.scale * 0.5, 2});
    tester.run("prt", {2, fixture.scale + 1.0, 2});
    // The last two actions are distinguished
    tester.run("psrep", {2, 2.5 * fixture.scale, 0.5, 2, 2}, 2);

    const auto complement = wrongAutomaton.complement(fixture.alphabet);
    std::cout << complement << std::endl;

    BOOST_CHECK_EQUAL(wrongAutomaton.stateSize() + 1, complement.stateSize());
    auto complementRunner = TimedAutomatonRunner{complement};
    ManualEqTester complementTester{correctRunner, complementRunner};
    complementTester.run("prprp", {2, fixture.scale * 0.5, 2, fixture.scale * 0.5, 2}, 6);
    complementTester.run("prt", {2, fixture.scale + 1.0, 2}, 4);
    // complementTester.run("psrep", {2, 2.5 * fixture.scale, 0.5, 2, 2}, 6);
    TimedAutomaton intersection;
    boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
    intersectionTA(fixture.targetAutomaton, wrongAutomaton.complement(fixture.alphabet), intersection, toIState);
    // Confirm that the intersection is non-empty.
    auto intersectionRunner = TimedAutomatonRunner{intersection};
    BOOST_CHECK(!intersectionRunner.step(2.0));
    BOOST_CHECK(!intersectionRunner.step('p'));
    BOOST_CHECK(!intersectionRunner.step(2.5 * fixture.scale));
    BOOST_CHECK(!intersectionRunner.step('s'));
    BOOST_CHECK(!intersectionRunner.step(0.5));
    BOOST_CHECK(!intersectionRunner.step('r'));
    BOOST_CHECK(!intersectionRunner.step(2.0));
    BOOST_CHECK(intersectionRunner.step('e'));
    BOOST_CHECK(intersectionRunner.step(2.0));
    BOOST_CHECK(intersectionRunner.step('p'));

    // Verify that the intersection is non-empty.
    ZoneAutomaton za;
    ta2za(intersection, za);
    BOOST_CHECK(za.sample().has_value());

    auto counterExampleOpt = oracle.findCounterExample(wrongAutomaton);
    BOOST_CHECK(counterExampleOpt);
  }

  BOOST_FIXTURE_TEST_CASE(queryWithUnobservableHypothesis, FixtureWithUnobservable) {
    auto oracle = ComplementTimedAutomataEquivalenceOracle{this->automaton, this->complementAutomaton, {'a'}};

    BOOST_CHECK(!oracle.findCounterExample(this->automatonWithOneUnobservable));
    BOOST_CHECK(!oracle.findCounterExample(this->automatonWithTwoUnobservable));
  }

  struct UnbalancedHypothesis20221219OracleFixture : public UnbalancedHypothesis20221219Fixture, public UnbalancedFixture{
    ComplementTimedAutomataEquivalenceOracle oracle;

    UnbalancedHypothesis20221219OracleFixture() : UnbalancedHypothesis20221219Fixture(), UnbalancedFixture(1), oracle(this->targetAutomaton, this->complementTargetAutomaton, this->alphabet) {}
  };

  BOOST_FIXTURE_TEST_CASE(queryUnbalancedHypothesis20221219, UnbalancedHypothesis20221219OracleFixture) {
    BOOST_CHECK(!oracle.subset(this->hypothesis));
    BOOST_CHECK(!oracle.superset(this->hypothesis));
    BOOST_CHECK(!oracle.findCounterExample(this->hypothesis));
  }
BOOST_AUTO_TEST_SUITE_END()
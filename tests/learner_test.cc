/**
 * @author Masaki Waga
 * @date 2022/03/21.
 */

#include <boost/test/unit_test.hpp>
#include <utility>

#include "../include/learner.hh"
#include "../include/timed_automaton_runner.hh"
#include "../include/timed_automata_equivalence_oracle.hh"

#include "simple_automaton_fixture.hh"
#include "small_light_automaton_fixture.hh"
#include "light_automaton_fixture.hh"

#include "manual_eq_tester.hh"

BOOST_AUTO_TEST_SUITE(LearnerTest)

  using namespace learnta;

  struct SimpleAutomatonOracleFixture : public SimpleAutomatonFixture, ComplementSimpleAutomatonFixture {
    std::unique_ptr<learnta::SymbolicMembershipOracle> memOracle;
    std::unique_ptr<learnta::EquivalenceOracle> eqOracle;
    const std::vector<Alphabet> alphabet = {'a'};

    SimpleAutomatonOracleFixture() {
      auto runner = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner{this->automaton});
      this->memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(runner));
      this->eqOracle = std::unique_ptr<learnta::EquivalenceOracle>(
              new learnta::ComplementTimedAutomataEquivalenceOracle{automaton, complementAutomaton, alphabet});
    }
  };

  struct SmallLightAutomatonOracleFixture : public SmallLightAutomatonFixture {
    std::unique_ptr<learnta::SymbolicMembershipOracle> memOracle;
    std::unique_ptr<learnta::EquivalenceOracle> eqOracle;

    SmallLightAutomatonOracleFixture() {
      auto runner = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner{this->targetAutomaton});
      this->memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(runner));
      this->eqOracle = std::unique_ptr<learnta::EquivalenceOracle>(
              new learnta::ComplementTimedAutomataEquivalenceOracle{targetAutomaton,
                                                                    complementTargetAutomaton,
                                                                    alphabet});
    }
  };

  struct LightAutomatonOracleFixture : public LightAutomatonFixture {
    std::unique_ptr<learnta::SymbolicMembershipOracle> memOracle;
    std::unique_ptr<learnta::EquivalenceOracle> eqOracle;

    explicit LightAutomatonOracleFixture(int scale = 5) : LightAutomatonFixture(scale) {
      auto runner = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner{this->targetAutomaton});
      this->memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(runner));
      this->eqOracle = std::unique_ptr<learnta::EquivalenceOracle>(
              new learnta::ComplementTimedAutomataEquivalenceOracle{targetAutomaton,
                                                                    complementTargetAutomaton,
                                                                    alphabet});
    }
  };

  BOOST_FIXTURE_TEST_CASE(simpleDTA, SimpleAutomatonOracleFixture) {
    Learner learner{this->alphabet, std::move(this->memOracle), std::move(this->eqOracle)};
    const auto result = learner.run();
    // learner.printStatistics(std::cout);
    BOOST_CHECK_EQUAL(2, result.stateSize());
  }

  BOOST_FIXTURE_TEST_CASE(smallLight, SmallLightAutomatonOracleFixture) {
    Learner learner{this->alphabet, std::move(this->memOracle), std::move(this->eqOracle)};
    const auto result = learner.run();
    learner.printStatistics(std::cout);
    BOOST_CHECK_EQUAL(4, result.stateSize());

    // Manually test the equivalence
    auto correctRunner = TimedAutomatonRunner{this->targetAutomaton};
    auto runner = TimedAutomatonRunner{result};
    ManualEqTester tester{correctRunner, runner};
    tester.run("srsrs", {2, this->scale * 0.5, 2, this->scale * 0.5, 2});
    tester.run("sr", {2, this->scale + 1.0});
  }

  BOOST_FIXTURE_TEST_CASE(light, LightAutomatonOracleFixture) {
    Learner learner{this->alphabet, std::move(this->memOracle), std::move(this->eqOracle)};
    const auto result = learner.run();
    learner.printStatistics(std::cout);
    std::cout << result << std::endl;
    BOOST_CHECK_EQUAL(5, learner.numEqQueries());
    BOOST_CHECK_EQUAL(6, result.stateSize());

    // Manually test the equivalence
    auto correctRunner = TimedAutomatonRunner{this->targetAutomaton};
    auto runner = TimedAutomatonRunner{result};
    ManualEqTester tester{correctRunner, runner};
    tester.run("prprp", {2, this->scale * 0.5, 2, this->scale * 0.5, 2});
    tester.run("prt", {2, this->scale + 1.0, 2});
    tester.run("psrep", {2, 2.5 * this->scale, 0.5, 2, 2});
  }

  BOOST_AUTO_TEST_CASE(light19) {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);

    auto fixture = LightAutomatonOracleFixture{19};
    Learner learner{fixture.alphabet, std::move(fixture.memOracle), std::move(fixture.eqOracle)};
    const auto result = learner.run();
    learner.printStatistics(std::cout);
    std::cout << result << std::endl;
    BOOST_CHECK_EQUAL(5, learner.numEqQueries());
    BOOST_CHECK_EQUAL(5, result.stateSize());

    // Manually test the equivalence
    auto correctRunner = TimedAutomatonRunner{fixture.targetAutomaton};
    auto runner = TimedAutomatonRunner{result};
    ManualEqTester tester{correctRunner, runner};
    tester.run("prprp", {2, fixture.scale * 0.5, 2, fixture.scale * 0.5, 2});
    tester.run("prt", {2, fixture.scale + 1.0, 2});
    tester.run("psrep", {2, 2.5 * fixture.scale, 0.5, 2, 2});
  }

BOOST_AUTO_TEST_SUITE_END()

/**
 * @author Masaki Waga
 * @date 2022/03/21.
 */

#include <boost/test/unit_test.hpp>
#include <utility>

#include "../include/learner.hh"
#include "../include/timed_automaton_runner.hh"
#include "../include/sul.hh"
#include "../include/timed_automata_equivalence_oracle.hh"

#include "simple_automaton_fixture.hh"
#include "small_light_automaton_fixture.hh"
#include "light_automaton_fixture.hh"


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

    LightAutomatonOracleFixture() {
      auto runner = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner{this->targetAutomaton});
      this->memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(runner));
      this->eqOracle = std::unique_ptr<learnta::EquivalenceOracle>(
              new learnta::ComplementTimedAutomataEquivalenceOracle{targetAutomaton,
                                                                    complementTargetAutomaton,
                                                                    alphabet});
    }
  };

  struct ManualEqTester {
    TimedAutomatonRunner expected, hypothesis;

    ManualEqTester(TimedAutomatonRunner expected, TimedAutomatonRunner hypothesis) :
            expected(std::move(expected)), hypothesis(std::move(hypothesis)) {
    }
    void run(std::string word, std::vector<double> durations) {
      hypothesis.pre();
      expected.pre();
      for (int i = 0; i < word.size(); ++i) {
        BOOST_CHECK_EQUAL(expected.step(durations.at(i)), hypothesis.step(durations.at(i)));
        BOOST_CHECK_EQUAL(expected.step(word.at(i)), hypothesis.step(word.at(i)));
      }
      if (durations.size() > word.size())  {
        BOOST_CHECK_EQUAL(expected.step(durations.back()), hypothesis.step(durations.back()));
      }
      hypothesis.post();
      expected.post();
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
    BOOST_CHECK_EQUAL(6, result.stateSize());

    // Manually test the equivalence
    auto correctRunner = TimedAutomatonRunner{this->targetAutomaton};
    auto runner = TimedAutomatonRunner{result};
    ManualEqTester tester{correctRunner, runner};
    tester.run("srsrs", {2, this->scale * 0.5, 2, this->scale * 0.5, 2});
    tester.run("sr", {2, this->scale + 1.0});
  }

BOOST_AUTO_TEST_SUITE_END()

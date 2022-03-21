/**
 * @author Masaki Waga
 * @date 2022/03/21.
 */

#include <boost/test/unit_test.hpp>

#include "../include/learner.hh"
#include "../include/timed_automaton_runner.hh"
#include "../include/sul.hh"
#include "../include/timed_automata_equivalence_oracle.hh"

#include "simple_automaton_fixture.hh"


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
              new learnta::ComplementTimedAutomataEquivalenceOracle{this->complementAutomaton});
    }
  };

  BOOST_FIXTURE_TEST_CASE(simpleDTA, SimpleAutomatonOracleFixture) {
    Learner learner{this->alphabet, std::move(this->memOracle), std::move(this->eqOracle)};
    learner.run();
  }

BOOST_AUTO_TEST_SUITE_END()

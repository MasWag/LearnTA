/**
 * @author Masaki Waga
 * @date 2022/11/30.
 */

#include "../include/counterexample_analyzer.hh"

using namespace learnta;
#include <boost/test/unit_test.hpp>

#include "simple_automaton_fixture.hh"
#include "timed_automaton_runner.hh"

BOOST_AUTO_TEST_SUITE(CounterexampleAnalyzerTest)
  struct SimpleAutomatonOracleFixture : public SimpleAutomatonFixture {
    SimpleAutomatonOracleFixture() : SimpleAutomatonFixture(), oracle(std::make_unique<SULMembershipOracle>(std::make_unique<TimedAutomatonRunner>(this->automaton))) {}
    std::unique_ptr<MembershipOracle> oracle;
  };

  BOOST_FIXTURE_TEST_CASE(analyzeCEXFirstHypothesis, SimpleAutomatonOracleFixture) {
    const ForwardRegionalElementaryLanguage initial;
    std::vector<ElementaryLanguage> prefixes = {initial};
    std::vector<ElementaryLanguage> final = prefixes;
    std::vector<SingleMorphism> morphisms = {
            SingleMorphism{initial.successor(), initial, RenamingRelation{}},
            SingleMorphism{initial.successor('a'), initial, RenamingRelation{}}
    };
    const RecognizableLanguage hypothesis {prefixes, final, morphisms};
    const TimedWord cex {"a", {1.0, 0.0}};
    const auto result = analyzeCEX(cex, this->oracle, hypothesis);
    BOOST_CHECK_EQUAL("a", result.getWord());
    BOOST_CHECK_EQUAL(2, result.getDurations().size());
    BOOST_CHECK_EQUAL(0.5, result.getDurations().at(0));
    BOOST_CHECK_EQUAL(0.0, result.getDurations().at(1));
  }
BOOST_AUTO_TEST_SUITE_END()
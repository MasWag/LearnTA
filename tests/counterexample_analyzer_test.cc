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
  template<int T>
  struct SimpleAutomatonOracleFixture : public SimpleAutomatonFixtureT<T> {
    SimpleAutomatonOracleFixture() : SimpleAutomatonFixtureT<T>(), oracle(std::make_unique<SULMembershipOracle>(std::make_unique<TimedAutomatonRunner>(this->automaton))) {}
    std::unique_ptr<MembershipOracle> oracle;
  };

  BOOST_FIXTURE_TEST_CASE(analyzeCEXFirstHypothesis, SimpleAutomatonOracleFixture<1>) {
    const ForwardRegionalElementaryLanguage initial;
    std::vector<ElementaryLanguage> prefixes = {initial};
    std::vector<ElementaryLanguage> final = prefixes;
    std::vector<SingleMorphism> morphisms = {
            SingleMorphism{initial.successor(), initial, RenamingRelation{}},
            SingleMorphism{initial.successor('a'), initial, RenamingRelation{}}
    };
    const RecognizableLanguage hypothesis {prefixes, final, morphisms};
    const TimedWord cex {"a", {1.0, 0.0}};
    const auto result = analyzeCEX(cex, *this->oracle, hypothesis,
                                   {BackwardRegionalElementaryLanguage::fromTimedWord(TimedWord{})});
    const std::array<double, 2> expectedDurations = {0.5, 0.0};
    BOOST_TEST(result.has_value());
    BOOST_CHECK_EQUAL("a", result->getWord());
    BOOST_CHECK_EQUAL_COLLECTIONS(expectedDurations.begin(), expectedDurations.end(),
                                  result->getDurations().begin(), result->getDurations().end());
  }

  BOOST_FIXTURE_TEST_CASE(analyzeCEXFirstHypothesisScaleTwo, SimpleAutomatonOracleFixture<2>) {
    const ForwardRegionalElementaryLanguage initial;
    std::vector<ElementaryLanguage> prefixes = {initial};
    std::vector<ElementaryLanguage> final = prefixes;
    std::vector<SingleMorphism> morphisms = {
            SingleMorphism{initial.successor(), initial, RenamingRelation{}},
            SingleMorphism{initial.successor('a'), initial, RenamingRelation{}}
    };
    const RecognizableLanguage hypothesis {prefixes, final, morphisms};
    const TimedWord cex {"a", {2.0, 0.0}};
    const auto result = analyzeCEX(cex, *this->oracle, hypothesis,
                                   {BackwardRegionalElementaryLanguage::fromTimedWord(TimedWord{})});
    const std::array<double, 2> expectedDurations = {1.5, 0.0};
    BOOST_TEST(result.has_value());
    BOOST_CHECK_EQUAL("a", result->getWord());
    BOOST_CHECK_EQUAL_COLLECTIONS(expectedDurations.begin(), expectedDurations.end(),
                                  result->getDurations().begin(), result->getDurations().end());
  }
BOOST_AUTO_TEST_SUITE_END()
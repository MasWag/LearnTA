/**
 * @author Masaki Waga
 * @date 2022/11/01.
 */
#include <sstream>
#include <boost/test/unit_test.hpp>
#define protected public

#include "../include/single_morphism.hh"

BOOST_AUTO_TEST_SUITE(SingleMorphismTest)

  using namespace learnta;
  BOOST_AUTO_TEST_CASE(maps) {
    // {0 < t0 < 1, 0 < t1 < 1, 0 < t0 + t1 < 1}
    auto domainCondition = TimedCondition{Zone::top(4)};
    domainCondition.restrictLowerBound(0, 0, Bounds{0, false});
    domainCondition.restrictUpperBound(0, 0, Bounds{1, false});
    domainCondition.restrictLowerBound(0, 1, Bounds{0, false});
    domainCondition.restrictUpperBound(0, 1, Bounds{1, false});
    domainCondition.restrictLowerBound(1, 1, Bounds{0, false});
    domainCondition.restrictUpperBound(1, 1, Bounds{1, false});
    domainCondition.restrictLowerBound(2, 2, Bounds{0, true});
    domainCondition.restrictUpperBound(2, 2, Bounds{0, true});
    ElementaryLanguage domain = {"aa", domainCondition};

    // {0 < r0 < 1, r1 = 0, 0 < r2 < 1, 0 < r0 + r1 + r2 < 1}
    auto codomainCondition = TimedCondition{Zone::top(4)};
    codomainCondition.restrictLowerBound(0, 0, Bounds{0, false});
    codomainCondition.restrictUpperBound(0, 0, Bounds{1, false});
    codomainCondition.restrictLowerBound(1, 1, Bounds{0, true});
    codomainCondition.restrictUpperBound(1, 1, Bounds{0, true});
    codomainCondition.restrictLowerBound(2, 2, Bounds{0, false});
    codomainCondition.restrictUpperBound(2, 2, Bounds{1, false});
    codomainCondition.restrictLowerBound(0, 2, Bounds{0, false});
    codomainCondition.restrictUpperBound(0, 2, Bounds{1, false});
    ElementaryLanguage codomain = {"aa", codomainCondition};

    // E = t0 + t1 = r0 + r1 + r2.
    RenamingRelation renaming;
    renaming.emplace_back(0, 0);

    const auto morphism = SingleMorphism{domain, codomain, renaming};
    const auto expected = TimedWord{"aa", {0.25, 0, 0.5}};
    BOOST_CHECK_EQUAL(expected, morphism.maps(TimedWord{"aa", {0.5, 0.25, 0.0}}));
  }

  BOOST_AUTO_TEST_CASE(maps2) {
    // domain: (a, 2 <= T_{0, 0}  <= 2 && 4 < T_{0, 1}  < 5 && 2 < T_{1, 1}  < 3)
    auto domainCondition = TimedCondition{Zone::top(3)};
    domainCondition.restrictLowerBound(0, 0, Bounds{-2, true});
    domainCondition.restrictUpperBound(0, 0, Bounds{2, true});
    domainCondition.restrictLowerBound(0, 1, Bounds{-4, false});
    domainCondition.restrictUpperBound(0, 1, Bounds{5, false});
    domainCondition.restrictLowerBound(1, 1, Bounds{-2, false});
    domainCondition.restrictUpperBound(1, 1, Bounds{3, false});
    ElementaryLanguage domain = {"a", domainCondition};

    // codomain: (a, 2 <= T_{0, 0}  <= 2 && 4 <= T_{0, 1}  <= 4 && 2 <= T_{1, 1}  <= 2)
    auto codomainCondition = TimedCondition{Zone::top(3)};
    codomainCondition.restrictLowerBound(0, 0, Bounds{-2, true});
    codomainCondition.restrictUpperBound(0, 0, Bounds{2, true});
    codomainCondition.restrictLowerBound(0, 1, Bounds{-4, true});
    codomainCondition.restrictUpperBound(0, 1, Bounds{4, true});
    codomainCondition.restrictLowerBound(1, 1, Bounds{-2, true});
    codomainCondition.restrictUpperBound(1, 1, Bounds{2, true});
    ElementaryLanguage codomain = {"a", codomainCondition};

    // renaming: {}
    RenamingRelation renaming;

    const auto morphism = SingleMorphism{domain, codomain, renaming};
    const auto expected = TimedWord{"a", {2.0, 2.0}};
    BOOST_CHECK_EQUAL(expected, morphism.maps(TimedWord{"a", {2.0, 2.5}}));
  }

  BOOST_AUTO_TEST_CASE(maps20230101) {
    std::stringstream stream;
    auto domainCondition = TimedCondition{Zone::top(4)};
    domainCondition.restrictLowerBound(0, 0, Bounds{-1, false});
    domainCondition.restrictUpperBound(0, 0, Bounds{2, false});
    domainCondition.restrictLowerBound(0, 1, Bounds{-2, true});
    domainCondition.restrictUpperBound(0, 1, Bounds{2, true});
    domainCondition.restrictLowerBound(0, 2, Bounds{-2, false});
    domainCondition.restrictUpperBound(0, 2, Bounds{3, false});
    domainCondition.restrictLowerBound(1, 1, Bounds{0, false});
    domainCondition.restrictUpperBound(1, 1, Bounds{1, false});
    domainCondition.restrictLowerBound(1, 2, Bounds{-1, false});
    domainCondition.restrictUpperBound(1, 2, Bounds{2, false});
    domainCondition.restrictLowerBound(2, 2, Bounds{0, false});
    domainCondition.restrictUpperBound(2, 2, Bounds{1, false});
    ElementaryLanguage domain = {"ab", domainCondition};
    stream << domain;
    BOOST_CHECK_EQUAL("(ab, 1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 1 && 1 < T_{1, 2}  < 2 && -0 < T_{2, 2}  < 1)",
                      stream.str());
    stream.str("");

    auto codomainCondition = TimedCondition{Zone::top(4)};
    codomainCondition.restrictLowerBound(0, 0, Bounds{-1, false});
    codomainCondition.restrictUpperBound(0, 0, Bounds{2, false});
    codomainCondition.restrictLowerBound(0, 1, Bounds{-2, true});
    codomainCondition.restrictUpperBound(0, 1, Bounds{2, true});
    codomainCondition.restrictLowerBound(0, 2, Bounds{-2, false});
    codomainCondition.restrictUpperBound(0, 2, Bounds{3, false});
    codomainCondition.restrictLowerBound(1, 1, Bounds{0, false});
    codomainCondition.restrictUpperBound(1, 1, Bounds{1, false});
    codomainCondition.restrictLowerBound(1, 2, Bounds{-1, true});
    codomainCondition.restrictUpperBound(1, 2, Bounds{1, true});
    codomainCondition.restrictLowerBound(2, 2, Bounds{0, false});
    codomainCondition.restrictUpperBound(2, 2, Bounds{2, false});
    ElementaryLanguage codomain = {"ab", codomainCondition};
    stream << codomain;
    BOOST_CHECK_EQUAL("(ab, 1 < T_{0, 0}  < 2 && 2 <= T_{0, 1}  <= 2 && 2 < T_{0, 2}  < 3 && -0 < T_{1, 1}  < 1 && 1 <= T_{1, 2}  <= 1 && -0 < T_{2, 2}  < 1)",
                      stream.str());
    stream.str("");

    // renaming: {t0 == t'0}
    RenamingRelation renaming;
    renaming.emplace_back(0, 0);

    const auto morphism = SingleMorphism{domain, codomain, renaming};
    const auto result = morphism.maps(TimedWord{"ab", {1.5, 0.5, 0.75}});
    BOOST_TEST(codomain.contains(result));
    const auto expected = TimedWord{"ab", {1.75, 0.25, 0.75}};
    BOOST_CHECK_EQUAL(expected, result);
  }
BOOST_AUTO_TEST_SUITE_END()

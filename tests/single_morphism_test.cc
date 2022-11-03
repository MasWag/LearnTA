/**
 * @author Masaki Waga
 * @date 2022/11/01.
 */
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
    const auto expected = TimedWord{"aa", {0.25, 0, 0.25}};
    BOOST_CHECK_EQUAL(expected, morphism.maps(TimedWord{"aa", {0.5, 0.25, 0.0}}));
  }

BOOST_AUTO_TEST_SUITE_END()

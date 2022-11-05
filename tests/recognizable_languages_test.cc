/**
 * @author Masaki Waga
 * @date 2022/11/04.
 */
#include <boost/test/unit_test.hpp>
#include "../include/recognizable_languages.hh"

BOOST_AUTO_TEST_SUITE(RecognizableLanguagesTest)

  using namespace learnta;

  // The running example in our paper
  struct RecognizableLanguagesFixture {
    std::unique_ptr<RecognizableLanguage> recognizable;
    std::vector<SingleMorphism> morphisms;
    RecognizableLanguagesFixture() {
      std::vector<ElementaryLanguage> prefixes;
      std::vector<ElementaryLanguage> final;
      // (, 0 <= T_{0, 0}  <= 0)
      auto zero = ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"", {0}});
      // (, 0 < T_{0, 0}  < 1)
      auto lessThanOne = zero.successor();
      // (, 1 <= T_{0, 0}  <= 1)
      auto one = lessThanOne.successor();
      // Initial location
      prefixes.push_back(zero);
      prefixes.push_back(lessThanOne);
      prefixes.push_back(one);
      final.push_back(zero);
      final.push_back(lessThanOne);
      final.push_back(one);

      // Non-initial location
      auto oneA = one.successor('a');
      prefixes.push_back(one.successor('a'));
      prefixes.push_back(one.successor('a').successor());
      prefixes.push_back(one.successor('a').successor().successor());
      prefixes.push_back(one.successor('a').successor().successor().successor());

      morphisms.emplace_back(zero.successor('a'),
                             zero,
                             RenamingRelation{{std::make_pair(0ul,0ul)}});
      morphisms.emplace_back(lessThanOne.successor('a'),
                             lessThanOne,
                             RenamingRelation{{std::make_pair(0ul,0ul)}});
      morphisms.emplace_back(one.successor(),
                             one,
                             RenamingRelation{{}});
      morphisms.emplace_back(oneA.successor('a'),
                             zero,
                             RenamingRelation{{std::make_pair(1ul,0ul)}});
      morphisms.emplace_back(oneA.successor().successor('a'),
                             lessThanOne,
                             RenamingRelation{{std::make_pair(1ul,0ul)}});
      morphisms.emplace_back(oneA.successor().successor().successor('a'),
                             one,
                             RenamingRelation{{std::make_pair(1ul,0ul)}});
      morphisms.emplace_back(oneA.successor().successor().successor().successor('a'),
                             oneA.successor().successor().successor(),
                             RenamingRelation{});
      morphisms.emplace_back(oneA.successor().successor().successor().successor(),
                             oneA.successor().successor().successor(),
                             RenamingRelation{});

      recognizable = std::make_unique<RecognizableLanguage>(prefixes, final, morphisms);
    }
  };

  BOOST_FIXTURE_TEST_CASE(splitGreaterThanOne, RecognizableLanguagesFixture) {
    const auto word = TimedWord{"", {1.25}};
    const auto prefix = TimedWord{"", {1.25}};
    const auto suffix = TimedWord{"", {0}};
    const auto mapped = TimedWord{"", {1.0}};
    BOOST_CHECK(recognizable->split(word));
    const auto expectedTriple = RecognizableLanguage::SplitTriple{prefix, suffix, morphisms.at(2)};
    BOOST_CHECK_EQUAL(expectedTriple, recognizable->split(word).value());
    BOOST_CHECK_EQUAL(mapped, expectedTriple.apply());
  }

  BOOST_FIXTURE_TEST_CASE(splitGreaterThanOneA, RecognizableLanguagesFixture) {
    const auto word = TimedWord{"a", {1.25, 0}};
    const auto prefix = TimedWord{"", {1.25}};
    const auto suffix = TimedWord{"a", {0, 0}};
    const auto mapped = TimedWord{"a", {1.0, 0}};
    BOOST_CHECK(recognizable->split(word));
    const auto expectedTriple = RecognizableLanguage::SplitTriple{prefix, suffix, morphisms.at(2)};
    BOOST_CHECK_EQUAL(expectedTriple, recognizable->split(word).value());
    BOOST_CHECK_EQUAL(mapped, expectedTriple.apply());
  }

  BOOST_FIXTURE_TEST_CASE(oneAGreaterThanOne, RecognizableLanguagesFixture) {
    const auto word = TimedWord{"a", {1.0, 0.25}};
    BOOST_CHECK(recognizable->inPrefixes(word));
    BOOST_CHECK(!recognizable->split(word));
    BOOST_CHECK(!recognizable->contains(word));
  }

  BOOST_FIXTURE_TEST_CASE(greaterThanOne, RecognizableLanguagesFixture) {
    const auto word = TimedWord{"", {1.75}};
    const auto prefix = TimedWord{"", {1.75}};
    const auto suffix = TimedWord{"", {0}};
    const auto mapped = TimedWord{"", {1.0}};
    auto zero = ForwardRegionalElementaryLanguage::fromTimedWord(TimedWord{"", {0}});
    BOOST_CHECK(!zero.contains(word));
    BOOST_CHECK(!recognizable->inPrefixes(word));
    BOOST_CHECK(recognizable->split(word));
    const auto expectedTriple = RecognizableLanguage::SplitTriple{prefix, suffix, morphisms.at(2)};
    BOOST_CHECK_EQUAL(expectedTriple, recognizable->split(word).value());
    BOOST_CHECK(recognizable->contains(word));
  }

  BOOST_FIXTURE_TEST_CASE(greaterThanTwo, RecognizableLanguagesFixture) {
    const auto word = TimedWord{"", {2.25}};
    const auto prefix = TimedWord{"", {1.5}};
    const auto suffix = TimedWord{"", {0.75}};
    const auto mapped = TimedWord{"", {1.75}};
    BOOST_CHECK(!recognizable->inPrefixes(word));
    BOOST_CHECK(recognizable->split(word));
    const auto expectedTriple = RecognizableLanguage::SplitTriple{prefix, suffix, morphisms.at(2)};
    BOOST_CHECK_EQUAL(expectedTriple, recognizable->split(word).value());
    BOOST_CHECK(recognizable->contains(word));
  }


  BOOST_FIXTURE_TEST_CASE(contains, RecognizableLanguagesFixture) {
    BOOST_CHECK(recognizable->contains(TimedWord{"", {0}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"", {0.5}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"", {1.0}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"", {1.25}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"", {3.0}}));

    BOOST_CHECK(recognizable->contains(TimedWord{"a", {0, 0}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"a", {0.5, 0}}));
    BOOST_CHECK(!recognizable->contains(TimedWord{"a", {1.0, 0}}));
    BOOST_CHECK(!recognizable->contains(TimedWord{"a", {1.25, 0}}));
    BOOST_CHECK(!recognizable->contains(TimedWord{"a", {3.0, 0}}));

    BOOST_CHECK(recognizable->contains(TimedWord{"a", {0, 0.25}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"a", {0.5, 0.25}}));
    BOOST_CHECK(!recognizable->contains(TimedWord{"a", {1.0, 0.25}}));
    BOOST_CHECK(!recognizable->contains(TimedWord{"a", {1.25, 0.25}}));
    BOOST_CHECK(!recognizable->contains(TimedWord{"a", {3.0, 0.25}}));

    BOOST_CHECK(recognizable->contains(TimedWord{"aa", {0, 0, 0}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"aa", {0.5, 0, 0}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"aa", {1.0, 0, 0}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"aa", {1.25, 0, 0}}));
    BOOST_CHECK(recognizable->contains(TimedWord{"aa", {3.0, 0, 0}}));
  }
BOOST_AUTO_TEST_SUITE_END()

/**
 * @author Masaki Waga
 * @date 2022/11/03.
 */

#pragma once

#include <utility>
#include <vector>
#include <ostream>
#include "single_morphism.hh"
#include "forward_regional_elementary_language.hh"

namespace learnta {
  /*!
   * @brief Recognizable timed language in [MP04]
   */
  class RecognizableLanguage {
  private:
    std::vector<ElementaryLanguage> prefixes;
    std::vector<ElementaryLanguage> final;
    std::vector<SingleMorphism> morphisms;

    [[nodiscard]] TimedWord maps(const TimedWord &word) const {
      if (this->inPrefixes(word)) {
        return word;
      } else {
        return split(word)->apply();
      }
    }

  public:
    RecognizableLanguage(std::vector<ElementaryLanguage> prefixes,
                         std::vector<ElementaryLanguage> final,
                         std::vector<SingleMorphism> morphisms) : prefixes(std::move(prefixes)),
                                                                  final(std::move(final)),
                                                                  morphisms(std::move(morphisms)) {
      // Assert the precondition
      assert(std::all_of(this->final.begin(), this->final.end(), [&] (const ElementaryLanguage &finalLanguage) {
        return std::find(this->prefixes.begin(), this->prefixes.end(), finalLanguage) != this->prefixes.end();
      }));
    }

    /*!
     * @brief Returns if the given timed word is in the prefixes
     */
    [[nodiscard]] bool inPrefixes(const TimedWord &word) const {
      return std::any_of(prefixes.begin(), prefixes.end(), [&] (const ElementaryLanguage &language) {
        return language.contains(word);
      });
    }

    /*!
     * @brief Returns if the given timed word is in the final prefixes
     */
    [[nodiscard]] bool isFinal(const TimedWord &word) const {
      return std::any_of(final.begin(), final.end(), [&] (const ElementaryLanguage &language) {
        return language.contains(word);
      });
    }

    struct SplitTriple {
      TimedWord prefix;
      TimedWord suffix;
      SingleMorphism morphism;
      [[nodiscard]] TimedWord apply() const {
        return morphism.maps(prefix) + suffix;
      }

      bool operator==(const SplitTriple &rhs) const {
        return prefix == rhs.prefix &&
               suffix == rhs.suffix &&
               morphism == rhs.morphism;
      }

      bool operator!=(const SplitTriple &rhs) const {
        return !(rhs == *this);
      }

      friend std::ostream &operator<<(std::ostream &os, const SplitTriple &triple) {
        os << "prefix: " << triple.prefix << " suffix: " << triple.suffix << " morphism: " << triple.morphism;
        return os;
      }
    };

    /*!
     * @brief Split the given timed word with the morphism
     */
    [[nodiscard]] std::optional<SplitTriple> split(const TimedWord &word) const {
      // Make the regional elementary language containing word
      const auto regionalElementary = ForwardRegionalElementaryLanguage::fromTimedWord(word);
      // Make the prefixes of it
      const auto elemPrefixes = regionalElementary.prefixes();
      // Find the corresponding morphism based on the prefixes
      const auto it = std::find_if(this->morphisms.begin(), this->morphisms.end(), [&] (const auto &morphism) {
        return std::any_of(elemPrefixes.begin(), elemPrefixes.end(), [&] (const auto& prefix) {
          return morphism.isDomain(prefix);
        });
      });
      if (it == this->morphisms.end()) {
        return std::nullopt;
      }
      // When we do not have to split
      if (it->getDomain().contains(word)) {
        return SplitTriple {word, TimedWord{"", {0}}, *it};
      }
      // Split word using the prefix
      auto prefixDomainTimedCondition = it->getDomain().getTimedCondition();
      std::vector<double> prefixDurations;
      prefixDurations.reserve(it->getDomain().wordSize());
      for (int i = 0; i < it->getDomain().wordSize(); ++i) {
        prefixDomainTimedCondition.restrictUpperBound(i, i, Bounds{word.getDurations().at(i), true});
        prefixDomainTimedCondition.restrictLowerBound(i, i, Bounds{-word.getDurations().at(i), true});
        prefixDurations.push_back(word.getDurations().at(i));
      }
      const auto upperBound = prefixDomainTimedCondition.getUpperBound(it->getDomain().wordSize(), it->getDomain().wordSize());
      if (Bounds{word.getDurations().at(it->getDomain().wordSize()), true} <= upperBound) {
        // We use the duration in the given timed word if it is in the bound
        prefixDurations.push_back(word.getDurations().at(it->getDomain().wordSize()));
      } else if (upperBound.second){
        // We use the upper bound if it is non-strict
        prefixDurations.push_back(upperBound.first);
      } else {
        // We sample a valuation if there is no longest duration
        const auto tmpPrefix = ElementaryLanguage{it->getDomain().getWord(), prefixDomainTimedCondition}.sample();
        prefixDurations.push_back(tmpPrefix.getDurations().back());
      }
      const auto prefix = TimedWord{it->getDomain().getWord(), prefixDurations};
      // return the SplitTriple
      return SplitTriple {prefix, word.getSuffix(prefix), *it};
    }

    /*!
     * @brief Returns if this recognizable timed language contains the given timed word
     */
    [[nodiscard]] bool contains(TimedWord word) const {
      while (!this->inPrefixes(word)) {
        word = this->maps(word);
      }
      return this->isFinal(word);
    }
  };
}
/**
 * @author Masaki Waga
 * @date 2022/11/01.
 */

#pragma once

#include <utility>
#include <ostream>

#include "elementary_language.hh"
#include "renaming_relation.hh"

namespace learnta {
  /*!
   * @brief Morphism from an external elementary language to an internal one.
   */
  class SingleMorphism {
  private:
    ElementaryLanguage domain, codomain;
    RenamingRelation renaming;
  public:
    /*!
     * @param[in] domain The domain of the morphism
     * @param[in] codomain The codomain of the morphism
     * @param[in] renaming The renaming relation defining the morphism
     */
    SingleMorphism(ElementaryLanguage domain,
                   ElementaryLanguage codomain,
                   RenamingRelation renaming): domain(std::move(domain)), codomain(std::move(codomain)), renaming(std::move(renaming)) {}

    /*!
     * @brief Check if the given timed word is in the domain of this morphism
     */
    [[nodiscard]] bool inDomain(const TimedWord &word) const {
      return this->domain.contains(word);
    }

    /*!
     * @brief Check if the given timed word is in the domain of this morphism
     */
    [[nodiscard]] bool isDomain(const ElementaryLanguage &elementaryLanguage) const {
      return this->domain == elementaryLanguage;
    }

    [[nodiscard]] const ElementaryLanguage &getDomain() const {
      return domain;
    }

    /*!
     * @brief Apply this single morphism to the given timed word.
     *
     * @pre this->inDomain(word)
     */
    [[nodiscard]] TimedWord maps(const TimedWord &word) const {
      assert(this->inDomain(word));
      // Juxtapose the timed conditions to constrain with the renaming relation
      auto juxtaposedCondition = TimedCondition{word.getDurations()} ^ codomain.getTimedCondition();
      juxtaposedCondition.addRenaming(this->renaming);
      juxtaposedCondition.canonize();
      assert(juxtaposedCondition.isSatisfiableNoCanonize());
      // Sample a value from the juxtaposed timed condition
      const auto values = juxtaposedCondition.sample();
      std::vector<double> durations;
      durations.resize(codomain.wordSize() + 1);
      for (int i = static_cast<int>(codomain.wordSize()); i >= 0; --i) {
        if (i == codomain.wordSize()) {
          durations.at(i) = values.back();
        } else {
          durations.at(i) = values.at(i + 1 + domain.wordSize()) - values.at(i + 2 + domain.wordSize());
        }
      }

      return TimedWord{this->codomain.getWord(), durations};
    }

    bool operator==(const SingleMorphism &rhs) const {
      return domain == rhs.domain &&
             codomain == rhs.codomain &&
             renaming == rhs.renaming;
    }

    bool operator!=(const SingleMorphism &rhs) const {
      return !(rhs == *this);
    }

    friend std::ostream &operator<<(std::ostream &os, const SingleMorphism &morphism) {
      os << "domain: " << morphism.domain << " codomain: " << morphism.codomain << " renaming: " << morphism.renaming;
      return os;
    }
  };
}
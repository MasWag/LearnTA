/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#pragma once

#include "elementary_language.hh"
#include "sul.hh"

namespace learnta {
  /*!
   * @brief The oracle to answer symbolic membership queries
   */
  class SymbolicMembershipOracle {
  private:
    std::unique_ptr<SUL> sul;

    [[nodiscard]] bool membership(const TimedWord &timedWord) {
      sul->pre();
      std::string word = timedWord.getWord();
      std::vector<double> duration = timedWord.getDurations();
      bool result = sul->step(duration[0]);
      for (int i = 0; i < timedWord.wordSize(); i++) {
        sul->step(word[i]);
        result = sul->step(duration[i + 1]);
      }
      sul->post();

      return result;
    }

    [[nodiscard]] bool included(const ElementaryLanguage &elementary) {
      return this->membership(elementary.sample());
    }

  public:
    explicit SymbolicMembershipOracle(std::unique_ptr<SUL>&& sul) : sul(std::move(sul)) {}

    /*!
     * @brief Make a symbolic membership query
     *
     * @returns std::nullopt if the answer is bottom
     * @returns The timed condition otherwise
     */
    std::optional<std::list<TimedCondition>> query(const ElementaryLanguage &elementary) {
      std::list<ElementaryLanguage> includedLanguages;
      bool allIncluded = true;
      std::vector<ElementaryLanguage> simpleVec;
      elementary.enumerate(simpleVec);
      // Check if each of the simple elementary language is in the target language
      for (const auto &simple: simpleVec) {
        if (this->included(simple)) {
          includedLanguages.push_back(simple);
        } else {
          allIncluded = false;
        }
      }

      if (includedLanguages.empty()) {
        return std::nullopt;
      } else if (allIncluded) {
        return std::make_optional<std::list<TimedCondition>>({elementary.getTimedCondition()});
      } else {
        auto convexHull = ElementaryLanguage::convexHull(includedLanguages);
        // Check if the convex hull is the exact union.
        std::vector<ElementaryLanguage> result;
        convexHull.enumerate(result);
        if (result.size() == includedLanguages.size()) {
          // When the convex hull is the exact union
          return std::make_optional<std::list<TimedCondition>>({convexHull.getTimedCondition()});
        } else {
          // When the convex hull is an overapproximation
          includedLanguages = ElementaryLanguage::reduce(std::move(includedLanguages));
          std::list<TimedCondition> resultList;
          resultList.resize(includedLanguages.size());
          std::transform(std::make_move_iterator(includedLanguages.begin()),
                         std::make_move_iterator(includedLanguages.end()), resultList.begin(),
                         [](auto && e) {
            return e.getTimedCondition();
          });

          return std::make_optional(resultList);
        }
      }
    }
  };
}
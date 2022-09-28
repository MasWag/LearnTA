/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#pragma once

#include "elementary_language.hh"
#include "sul.hh"
#include "timed_condition_set.hh"

namespace learnta {
  /*!
   * @brief The oracle to answer symbolic membership queries
   */
  class SymbolicMembershipOracle {
  private:
    std::unique_ptr<SUL> sul;
    boost::unordered_map<TimedWord, bool> membershipCache;

    [[nodiscard]] bool membership(const TimedWord &timedWord) {
      auto it = this->membershipCache.find(timedWord);
      if (it != membershipCache.end()) {
        return it->second;
      }
      sul->pre();
      std::string word = timedWord.getWord();
      std::vector<double> duration = timedWord.getDurations();
      bool result = sul->step(duration[0]);
      for (int i = 0; i < timedWord.wordSize(); i++) {
        sul->step(word[i]);
        result = sul->step(duration[i + 1]);
      }
      sul->post();
      this->membershipCache[timedWord] = result;

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
     * @returns A list representing the resulting timed conditions
     */
    TimedConditionSet query(const ElementaryLanguage &elementary) {
      std::list<ElementaryLanguage> includedLanguages;
      bool allIncluded = true;
      // Check if each of the simple elementary language is in the target language
      for (const auto &simple: elementary.enumerate()) {
        if (this->included(simple)) {
          includedLanguages.push_back(simple);
        } else {
          allIncluded = false;
        }
      }

      // Simplify the result
      if (includedLanguages.empty()) {
        return TimedConditionSet::bottom();
      } else if (allIncluded) {
        return TimedConditionSet{elementary.getTimedCondition()};
      } else {
        auto convexHull = ElementaryLanguage::convexHull(includedLanguages);
        // Check if the convex hull is the exact union.
        if (convexHull.enumerate().size() == includedLanguages.size()) {
          // When the convex hull is the exact union
          return TimedConditionSet{convexHull.getTimedCondition()};
        } else {
          // When the convex hull is an overapproximation
          return TimedConditionSet::reduce(std::move(includedLanguages));
        }
      }
    }

    [[nodiscard]] std::size_t count() const {
      return this->sul->count();
    }
  };
}
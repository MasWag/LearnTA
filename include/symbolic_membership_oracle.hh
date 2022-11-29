/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#pragma once

#include "elementary_language.hh"
#include "sul.hh"
#include "membership_oracle.hh"
#include "timed_condition_set.hh"

namespace learnta {
  /*!
   * @brief The oracle to answer symbolic membership queries
   */
  class SymbolicMembershipOracle final : public MembershipOracle {
  private:
    std::unique_ptr<MembershipOracle> membershipOracle;
    boost::unordered_map<ElementaryLanguage, TimedConditionSet> cache;

    [[nodiscard]] bool included(const ElementaryLanguage &elementary) {
      return this->membershipOracle->answerQuery(elementary.sample());
    }

  public:
    explicit SymbolicMembershipOracle(std::unique_ptr<SUL>&& sul) : membershipOracle(
            std::make_unique<MembershipOracleCache>(std::make_unique<SULMembershipOracle>(std::move(sul)))) {}

    /*!
     * @brief Make a symbolic membership query
     *
     * @returns A list representing the resulting timed conditions
     */
    TimedConditionSet query(const ElementaryLanguage &elementary) {
      auto it = cache.find(elementary);
      if (it != cache.end()) {
        return it->second;
      }
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
        cache[elementary] = TimedConditionSet::bottom();
        return cache[elementary];
      } else if (allIncluded) {
        cache[elementary] = TimedConditionSet{elementary.getTimedCondition()};
        return cache[elementary];
      } else {
        auto convexHull = ElementaryLanguage::convexHull(includedLanguages);
        // Check if the convex hull is the exact union.
        if (convexHull.enumerate().size() == includedLanguages.size()) {
          // When the convex hull is the exact union
          cache[elementary] = TimedConditionSet{convexHull.getTimedCondition()};
          return cache[elementary];
        } else {
          // When the convex hull is an overapproximation
          cache[elementary] = TimedConditionSet::reduce(std::move(includedLanguages));
          return cache[elementary];
        }
      }
    }

    [[nodiscard]] std::size_t count() const override {
      return this->membershipOracle->count();
    }

    [[nodiscard]] bool answerQuery(const TimedWord &timedWord) override {
      return this->membershipOracle->answerQuery(timedWord);
    }
  };
}
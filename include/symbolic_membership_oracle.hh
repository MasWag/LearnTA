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

    [[nodiscard]] bool membership(const TimedWord& timedWord) {
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

    [[nodiscard]] bool included(const ElementaryLanguage& elementary) {
      return this->membership(elementary.sample());
    }
  public:
    explicit SymbolicMembershipOracle(std::unique_ptr<SUL> sul) : sul(std::move(sul)) {}

    std::optional<TimedCondition> query(const ElementaryLanguage& elementary) {
      std::vector<ElementaryLanguage> includedLanguages;
      bool allIncluded = true;
      std::vector<ElementaryLanguage> simpleVec;
      elementary.enumerate(simpleVec);
      for (const auto &simple: simpleVec) {
        if (this->included(simple)) {
          includedLanguages.push_back(simple);
        } else {
          allIncluded = false;
        }

        if (includedLanguages.empty()) {
          return std::nullopt;
        } else if (allIncluded) {
          return std::make_optional(elementary.getTimedCondition());
        } else {
          abort();
        }
      }
    }
  };
}
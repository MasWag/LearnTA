/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#pragma once

#include <string>
#include <utility>

#include "timed_condition.hh"
#include "timed_word.hh"

namespace learnta {
  /*!
   * @brief An elementary language
   *
   * An elementary language is a timed language \f$\{ \tau_0 a_1 \tau_1 a_2 \tau_2 \dots \tau_{n-1} a_n \tau_{n} \mid \tau_0, \tau_1, \dots, \tau_n \models \Lambda \}\f$, where \f$a_1 a_2 \dots a_n \f$ is word and \f$\Lambda\f$ is timedCondition.
   * @invariant word.size() + 1 == timedCondition.size()
   */
  class ElementaryLanguage {
  protected:
    std::string word;
    TimedCondition timedCondition;
  public:
    ElementaryLanguage() = default;

    ElementaryLanguage(std::string word, TimedCondition timedCondition) :
            word(std::move(word)), timedCondition(std::move(timedCondition)) {}

    /*!
     * @brief Construct the empty elementary language containing only 0.
     */
    static ElementaryLanguage empty() {
      ElementaryLanguage elementary;
      elementary.word = "";
      elementary.timedCondition = TimedCondition::empty();
      return elementary;
    }

    /*!
     * @brief Construct a convex-hull of the given timed conditions
     */
    static ElementaryLanguage convexHull(const std::list<ElementaryLanguage>& elementaryLanguages) {
      if (elementaryLanguages.empty()) {
        return empty();
      }
      ElementaryLanguage result = elementaryLanguages.front();
      for (const auto& elementary: elementaryLanguages) {
        assert(elementary.word == result.word);
        result.timedCondition.convexHullAssign(elementary.timedCondition);
      }

      return result;
    }

    /*!
     * @brief Returns if this elementary language is simple
     */
    [[nodiscard]] bool isSimple() const {
      return this->timedCondition.isSimple();
    }

    /*!
     * @brief Returns the number of the events in this elementary language
     */
    [[nodiscard]] std::size_t wordSize() const {
      return this->word.size();
    }

    /*!
     * @brief Concatenate two elementary languages
     */
    ElementaryLanguage operator+(const ElementaryLanguage &another) const {
      return {this->word + another.word, this->timedCondition + another.timedCondition};
    }

    /*!
     * @brief Make a vector of simple elementary languages in this elementary language
     */
    void enumerate(std::vector<ElementaryLanguage> &result) const {
      if (this->isSimple()) {
        result = {*this};
        return;
      }
      std::vector<TimedCondition> simpleTimedConditions;
      this->timedCondition.enumerate(simpleTimedConditions);
      result.resize(simpleTimedConditions.size());
      std::transform(simpleTimedConditions.begin(), simpleTimedConditions.end(), result.begin(),
                     [&](const TimedCondition &cond) {
                       return ElementaryLanguage{this->word, cond};
                     });
    }

    /*!
     * @brief Return a timed word in this elementary language
     */
    [[nodiscard]] TimedWord sample() const {
      std::vector<double> durations;
      std::size_t N = this->wordSize() + 1;
      durations.resize(N);
      for (int i = 0; i < N; i++) {
        Bounds lowerBound = this->timedCondition.getLowerBound(i, i);
        Bounds upperBound = this->timedCondition.getUpperBound(i, i);
        if (isPoint(upperBound, lowerBound)) {
          durations[i] = upperBound.first;
        } else {
          double lower = -lowerBound.first;
          double upper = upperBound.first;
          double sum = 0;
          for (int j = i - 1; j >= 0; j--) {
            // sum = durations[j] + durations[j + 1] + ... + durations[i - 1]
            sum += durations[j];
            Bounds tmpLowerBound = this->timedCondition.getLowerBound(j, i);
            Bounds tmpUpperBound = this->timedCondition.getUpperBound(j, i);
            lower = std::max(lower, -tmpLowerBound.first - sum);
            upper = std::min(upper, tmpUpperBound.first - sum);
          }
          assert(lower <= upper);
          durations[i] = (lower + upper) * 0.5;
        }
      }

      return {this->word, durations};
    }

    void removeEqualityUpperBoundAssign() {
      timedCondition.removeEqualityUpperBoundAssign();
    };

    [[nodiscard]] const std::string &getWord() const {
      return word;
    }

    [[nodiscard]] const TimedCondition &getTimedCondition() const {
      return timedCondition;
    }
  };
}
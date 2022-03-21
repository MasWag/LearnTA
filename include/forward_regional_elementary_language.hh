/**
 * @author Masaki Waga
 * @date 2022/03/06.
 */

#pragma once

#include <string>
#include <utility>

#include "elementary_language.hh"
#include "fractional_order.hh"

namespace learnta {
  /*!
   * @brief A forward regional elementary language
   *
   * A forward regional elementary language is an elementary langauge \f$(u, \Lambda)\f$ with an order over the fractional parts of \f$\mathbb{T}_{0,N},\mathbb{T}_{1,N},\dots,\mathbb{T}_{N,N}\f$, where \f$\mathbb{T}_{i,N} = \tau_{i} + \tau_{i+1} \dots \tau_{N}\f$.
   *
   * @invariant elementary.wordSize() + 1 == fractionalOrder.size()
   */
  class ForwardRegionalElementaryLanguage : public ElementaryLanguage {
  protected:
    FractionalOrder fractionalOrder;
  public:
    //! @brief Construct the empty language
    ForwardRegionalElementaryLanguage() = default;

    ForwardRegionalElementaryLanguage(ElementaryLanguage elementary, FractionalOrder fractionalOrder) :
            ElementaryLanguage(std::move(elementary)), fractionalOrder(std::move(fractionalOrder)) {
      assert(this->fractionalOrder.getSize() == this->wordSize() + 1);
    }

    static ForwardRegionalElementaryLanguage fromTimedWord(const TimedWord &timedWord) {
      std::vector<double> fractionalPart, accumulatedDuration;
      fractionalPart.resize(timedWord.wordSize() + 1);
      accumulatedDuration.resize(timedWord.wordSize() + 1);
      accumulatedDuration.back() = timedWord.getDurations().back();
      fractionalPart.back() = timedWord.getDurations().back() - double(long(timedWord.getDurations().back()));
      for (int i = fractionalPart.size() - 2; i >= 0; --i) {
        accumulatedDuration.at(i) = accumulatedDuration.at(i + 1) + timedWord.getDurations().at(i);
        fractionalPart.at(i) = accumulatedDuration.at(i);
        fractionalPart.at(i) -= double(long(fractionalPart.at(i)));
      }

      return {ElementaryLanguage{timedWord.getWord(), TimedCondition{accumulatedDuration}},
              FractionalOrder{fractionalPart}};
    }

    /*!
     * @brief Construct the discrete successor
     */
    [[nodiscard]] ForwardRegionalElementaryLanguage successor(char action) const {
      return {{this->word + action, this->timedCondition.extendN()}, fractionalOrder.extendN()};
    }

    /*!
     * @brief Construct the continuous successor
     */
    [[nodiscard]] ForwardRegionalElementaryLanguage successor() const {
      return {{this->word, this->timedCondition.successor(fractionalOrder.successorVariables())},
              fractionalOrder.successor()};
    }

    /*!
     * @brief Return if there is \f$\mathbb{T}_{i,N} = c\f$.
     *
     * @pre the timed condition is simple
     */
    [[nodiscard]] bool hasEqualityN() const {
      // By simplicity of the timed condition, we can check only the one side
      return this->timedCondition.hasEqualityN();
    }

    /*!
     * @brief Return the immediate prefix if exists
     */
    [[nodiscard]] std::optional<ForwardRegionalElementaryLanguage> immediatePrefix() const {
      if (this->getWord().empty() && !this->getTimedCondition().hasPrefix()) {
        // When no prefix exists
        return std::nullopt;
      } else if (this->getTimedCondition().hasPrefix()) {
        // return the continuous prefix
        return std::make_optional(ForwardRegionalElementaryLanguage{
                {this->getWord(), this->getTimedCondition().prefix(this->fractionalOrder.predecessorVariables())},
                this->fractionalOrder.predecessor()});
      } else {
        // return the discrete prefix
        auto word = this->getWord();
        word.pop_back();
        return std::make_optional(ForwardRegionalElementaryLanguage{
                {word, this->timedCondition.removeN()}, this->fractionalOrder.removeN()});
      }
    }

    /*!
     * @brief Return the prefixes
     */
    [[nodiscard]] std::vector<ForwardRegionalElementaryLanguage> prefixes() const {
      std::list<ForwardRegionalElementaryLanguage> resultList;
      auto language = *this;
      resultList.push_front(language);
      auto next = language.immediatePrefix();

      while (next.has_value()) {
        language = *next;
        resultList.push_front(language);
        next = language.immediatePrefix();
      }

      std::vector<ForwardRegionalElementaryLanguage> result;
      result.resize(resultList.size());
      std::move(resultList.begin(), resultList.end(), result.begin());

      return result;
    }
  };
}

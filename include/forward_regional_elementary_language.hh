/**
 * @author Masaki Waga
 * @date 2022/03/06.
 */

#pragma once

#include <string>
#include <utility>

#include "elementary_language.hh"
#include "fractional_order.hh"
#include "backward_regional_elementary_language.hh"

namespace learnta {
  /*!
   * @brief A forward regional elementary language
   *
   * A forward regional elementary language is an elementary language \f$(u, \Lambda)\f$ with an order over the fractional parts of \f$\mathbb{T}_{0,N},\mathbb{T}_{1,N},\dots,\mathbb{T}_{N,N}\f$, where \f$\mathbb{T}_{i,N} = \tau_{i} + \tau_{i+1} \dots \tau_{N}\f$.
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

    ForwardRegionalElementaryLanguage(const ForwardRegionalElementaryLanguage &language) = default;

    ForwardRegionalElementaryLanguage(ForwardRegionalElementaryLanguage &&language) = default;

    ForwardRegionalElementaryLanguage &operator=(const ForwardRegionalElementaryLanguage &language) = default;

    ForwardRegionalElementaryLanguage &operator=(ForwardRegionalElementaryLanguage &&language) = default;

    /*!
     * @brief Construct the fractional elementary language containing the given timed word
     */
    static ForwardRegionalElementaryLanguage fromTimedWord(const TimedWord &timedWord) {
      std::vector<double> fractionalPart, accumulatedDuration;
      fractionalPart.resize(timedWord.wordSize() + 1);
      accumulatedDuration.resize(timedWord.wordSize() + 1);
      accumulatedDuration.back() = timedWord.getDurations().back();
      fractionalPart.back() = timedWord.getDurations().back() - std::floor(timedWord.getDurations().back());
      for (int i = static_cast<int>(fractionalPart.size()) - 2; i >= 0; --i) {
        accumulatedDuration.at(i) = accumulatedDuration.at(i + 1) + timedWord.getDurations().at(i);
        fractionalPart.at(i) = accumulatedDuration.at(i);
        fractionalPart.at(i) -= std::floor(fractionalPart.at(i));
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
     * @brief Construct the continuous successor
     */
    void successorAssign() {
      this->timedCondition.successorAssign(fractionalOrder.successorVariables());
      this->fractionalOrder.successorAssign();
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
     * @brief Return the prefixes in the shorter to the longer order
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

    /*!
     * @brief Return the suffix s such that this \subseteq p \cdot s
     *
     * @pre prefix is simple
     * @pre this is simple
     * @pre prefix is a prefix of this
     */
    [[nodiscard]] BackwardRegionalElementaryLanguage suffix(const ForwardRegionalElementaryLanguage &prefix) const {
      // Check the preconditions
      assert(prefix.isSimple());
      assert(this->isSimple());
      assert(this->word.compare(0, prefix.wordSize(), prefix.getWord()) == 0);
      const auto prefixWord = prefix.sample();
      ElementaryLanguage tmpLanguage = this->constrain(prefixWord);
      const auto fullWord = tmpLanguage.sample();

      const std::size_t suffixWordSize = this->wordSize() - prefix.wordSize();
      // Generate the word of the suffix
      const std::string suffixWord = this->word.substr(prefix.wordSize(), suffixWordSize);
      auto suffixDurations = fullWord.getDurations();
      suffixDurations.erase(suffixDurations.begin(), suffixDurations.begin() + prefix.wordSize());
      suffixDurations.front() -= prefixWord.getDurations().back();
      // Generate the elementary language containing the suffix
      const auto forward = fromTimedWord(TimedWord{suffixWord, suffixDurations});
      // Construct the fractional order
      std::vector<double> suffixDurationsFractional;
      suffixDurationsFractional.resize(suffixDurations.size());
      suffixDurationsFractional.front() = suffixDurations.front() - std::floor(suffixDurations.front());
      for (std::size_t i = 1; i < suffixDurationsFractional.size(); ++i) {
        suffixDurationsFractional.at(i) = suffixDurationsFractional.at(i - 1) + suffixDurations.at(i);
        suffixDurationsFractional.at(i) -= std::floor(suffixDurationsFractional.at(i));
      }
      assert(std::all_of(suffixDurationsFractional.begin(), suffixDurationsFractional.end(),
                         [](const double fractional) {
                           return 0 <= fractional && fractional < 1;
                         }));

      return BackwardRegionalElementaryLanguage{ElementaryLanguage{forward.getWord(), forward.getTimedCondition()},
                                                FractionalOrder(suffixDurationsFractional)};
    }

    [[nodiscard]] ForwardRegionalElementaryLanguage applyResets(const TATransition::Resets &resets) const {
      return fromTimedWord(ElementaryLanguage{this->word, this->timedCondition.applyResets(resets)}.sample());
    }

    /*!
     * @brief Return the language after applying the given reset
     *
     * @param newWord The word of the generated elementary language
     * @param resets The applies reset
     * @param targetClockSize The number of the clock variables in the target language
     *
     * @pre newWord.size() + 1 == targetClockSize
     * @post There is a clock valuation corresponding to a word in this language such that its value after reset corresponds to a word in the resulting language.
     */
    [[nodiscard]] ForwardRegionalElementaryLanguage applyResets(const std::string &newWord,
                                                                const TATransition::Resets &resets,
                                                                const std::size_t targetClockSize) const {
      assert(newWord.size() + 1 == targetClockSize);
      return fromTimedWord(ElementaryLanguage{newWord,
                                              this->timedCondition.applyResets(resets, targetClockSize)}.sample());
    }

    bool operator==(const ForwardRegionalElementaryLanguage &another) const {
      return this->getWord() == another.getWord() && this->getTimedCondition() == another.getTimedCondition() &&
             this->fractionalOrder == another.fractionalOrder;
    }

    std::ostream &print(std::ostream &os) const {
      os << "(" << this->getWord() << ", " << this->getTimedCondition() << ", " << this->fractionalOrder << ")";

      return os;
    }

    [[nodiscard]] std::size_t hash_value() const {
      return boost::hash_value(std::make_tuple(this->getWord(), this->getTimedCondition(),
                                               this->fractionalOrder.hash_value()));
    }
  };

  static inline std::ostream &operator<<(std::ostream &os, const learnta::ForwardRegionalElementaryLanguage &lang) {
    return lang.print(os);
  }

  inline std::size_t hash_value(learnta::ForwardRegionalElementaryLanguage const &lang) {
    return lang.hash_value();
  }
}

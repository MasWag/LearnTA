/**
 * @author Masaki Waga
 * @date 2022/03/07.
 */

#pragma once

#include <string>
#include <utility>

#include "elementary_language.hh"
#include "fractional_order.hh"

namespace learnta {
  /*!
   * @brief A back regional elementary language
   *
   * A back regional elementary language is an elementary language \f$(u, \Lambda)\f$ with an order over the fractional parts of \f$\mathbb{T}_{0,0},\mathbb{T}_{0,1},\dots,\mathbb{T}_{0,N}\f$, where \f$\mathbb{T}_{0,i} = \tau_{0} + \tau_{1} \dots \tau_{i}\f$.
   *
   * @invariant elementary.wordSize() + 1 == fractionalOrder.size()
   */
  class BackwardRegionalElementaryLanguage : public ElementaryLanguage {
  protected:
    FractionalOrder fractionalOrder;
  public:
    //! @brief Construct the empty language
    BackwardRegionalElementaryLanguage() = default;

    BackwardRegionalElementaryLanguage(ElementaryLanguage elementary, FractionalOrder fractionalOrder) :
            ElementaryLanguage(std::move(elementary)), fractionalOrder(std::move(fractionalOrder)) {}

    /*!
     * @brief Construct the discrete predecessor
     */
    [[nodiscard]] BackwardRegionalElementaryLanguage predecessor(char action) const {
      return {{action + this->word, this->timedCondition.extendZero()}, fractionalOrder.extendZero()};
    }

    /*!
     * @brief Construct the continuous predecessor
     */
    [[nodiscard]] BackwardRegionalElementaryLanguage predecessor() const {
      return {{this->word, this->timedCondition.predecessor(fractionalOrder.predecessorVariables())},
              fractionalOrder.predecessor()};
    }

    std::ostream &print(std::ostream &os) const {
      os << "(" << this->getWord() << ", " << this->getTimedCondition() << ", " << this->fractionalOrder;

      return os;
    }

    [[nodiscard]] const FractionalOrder &getFractionalOrder() const {
      return fractionalOrder;
    }

    bool operator==(const BackwardRegionalElementaryLanguage &another) const {
      return this->word == another.word && this->timedCondition == another.timedCondition &&
             this->fractionalOrder == another.fractionalOrder;
    }
  };

  static inline std::ostream &operator<<(std::ostream &os, const learnta::BackwardRegionalElementaryLanguage &lang) {
    return lang.print(os);
  }

  inline std::size_t hash_value(learnta::BackwardRegionalElementaryLanguage const &lang) {
    return boost::hash_value(std::make_tuple(lang.getWord(), lang.getTimedCondition(), lang.getFractionalOrder()));
  }
}

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
    ForwardRegionalElementaryLanguage() = default;

    ForwardRegionalElementaryLanguage(ElementaryLanguage elementary, FractionalOrder fractionalOrder) :
            ElementaryLanguage(std::move(elementary)), fractionalOrder(std::move(fractionalOrder)) {
      assert(this->fractionalOrder.size == this->wordSize() + 1);
    }

    /*!
     * @brief Construct the discrete successor
     */
    [[nodiscard]] ForwardRegionalElementaryLanguage successor (char action) const {
      return {{this->word + action, this->timedCondition.extendN()}, fractionalOrder.extendN()};
    }

    /*!
     * @brief Construct the continuous successor
     */
    [[nodiscard]] ForwardRegionalElementaryLanguage successor () const {
      return {{this->word, this->timedCondition.successor(fractionalOrder.successorVariables())},
              fractionalOrder.successor()};
    }
  };
}

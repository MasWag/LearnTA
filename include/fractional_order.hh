/**
 * @author Masaki Waga
 * @date 2022/03/05.
 */

#pragma once

#include <list>
#include <unordered_set>

#include "common_types.hh"

namespace learnta {
  /*!
   * @brief Order on the fractional part of the variables
   *
   * We implement this order by a list of sets of integers. For example, [{x1, x2}, {x3}, {x4}] represents 0 = x1 = x2 < x3 < x4.
   */
  class FractionalOrder {
  private:
    std::list<std::list<ClockVariables>> order;
    // The number of the variables
    std::size_t size;
  public:
    FractionalOrder() {
      order.push_front({0});
      size = 1;
    }

    /*!
     * @brief Return the variable to elapse
     */
     [[nodiscard]] std::list<ClockVariables> successorVariables() const {
       if (order.front().empty()) {
         return order.back();
       } else {
         return order.front();
       }
     }

    /*!
     * @brief Make it to be the successor
     */
    [[nodiscard]] FractionalOrder successor() const {
      FractionalOrder result = *this;
      if (order.front().empty()) {
        // If there is no variables equal to 0.
        std::swap(result.order.front(), result.order.back());
        auto lastIt = result.order.end();
        result.order.erase(--lastIt);
      } else {
        // If there are some variables equal to 0.
        result.order.emplace_front();
      }

      return result;
    }

    /*!
     * @brief Return the variable to backward-elapse
     */
    [[nodiscard]] std::list<ClockVariables> predecessorVariables() const {
      if (order.front().empty()) {
        auto it = order.begin();
        it++;
        return *(it);
      } else {
        return order.front();
      }
    }

    /*!
     * @brief Make it to be the predecessor
     */
    [[nodiscard]] FractionalOrder predecessor() const {
      FractionalOrder result = *this;
      if (order.front().empty()) {
        // If there is no variables equal to 0.
        result.order.pop_front();
      } else {
        // If there are some variables equal to 0.
        result.order.emplace_back();
        std::swap(result.order.front(), result.order.back());
      }

      return result;
    }

    /*!
     * @brief Add another variable \f$x_{n+1}\f$ such that \f$fractional(x_{n+1}) = 0\f$.
     */
    [[nodiscard]] FractionalOrder extendN() const {
      FractionalOrder result = *this;
      result.order.front().push_back(result.size++);
      return result;
    }

    /*!
     * @brief Rename each variable \f$x_i\f$ to \f$x_{i+1}\f$ and add \f$x_0\f$ such that \f$fractional(x_0) = 0\f$.
     */
    [[nodiscard]] FractionalOrder extendZero() const {
      FractionalOrder result = *this;

      for (auto &variables: result.order) {
        std::transform(variables.begin(), variables.end(), variables.begin(), [](auto variable) {
          return variable + 1;
        });
      }
      result.order.front().push_front(0);
      result.size++;
      return result;
    }
    //! @brief Returns the number of the variables
    [[nodiscard]] size_t getSize() const {
      return size;
    }
  };
}
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
     * @brief Make it to be the successor
     */
    void successor() {
      if (order.front().empty()) {
        // If there is no variables equal to 0.
        std::swap(this->order.front(), this->order.back());
        auto lastIt = this->order.end();
        this->order.erase(--lastIt);
      } else {
        // If there are some variables equal to 0.
        this->order.emplace_front();
      }
    }

    /*!
     * @brief Make it to be the predecessor
     */
    void predecessor() {
      if (order.front().empty()) {
        // If there is no variables equal to 0.
        order.pop_front();
      } else {
        // If there are some variables equal to 0.
        this->order.emplace_back();
        std::swap(this->order.front(), this->order.back());
      }
    }

    /*!
     * @brief Add another variable \f$x_{n+1}\f$ such that \f$x_n = x_{n+1}\f$.
     */
    void extendEq() {
      for (auto &variables: order) {
        // Since each list is ordered, size must be the last element if it belongs to variables.
        if (variables.back() == size - 1) {
          variables.push_back(size++);
          return;
        }
      }
    }

    /*!
     * @brief Rename each variable \f$x_i\f$ to \f$x_{i+1}\f$ and add \f$x_0\f$ such that \f$x_0 = x_1\f$.
     */
    void extendZero() {
      for (auto &variables: order) {
        std::transform(variables.begin(), variables.end(), variables.begin(), [](auto variable) {
          return variable + 1;
        });
        // Since each list is ordered, 1 must be the first element if it belongs to variables.
        if (variables.front() == 1) {
          variables.push_front(0);
        }
      }
    }
    //! @brief Returns the number of the variables
    [[nodiscard]] size_t getSize() const {
      return size;
    }
  };
}
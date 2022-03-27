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
     * @brief Construct a fractional order from a concrete vector of fractional parts.
     */
    explicit FractionalOrder(const std::vector<double> &fractionalParts) {
      std::vector<std::pair<double, ClockVariables>> fractionalPartsWithIndices;
      fractionalPartsWithIndices.resize(fractionalParts.size());
      for (int i = 0; i < fractionalParts.size(); ++i) {
        fractionalPartsWithIndices.at(i) = std::make_pair(fractionalParts.at(i), i);
      }
      std::sort(fractionalPartsWithIndices.begin(), fractionalPartsWithIndices.end());
      double currentFractionalPart = 0;
      order.emplace_back();
      for (const auto&[fractionalPart, index]: fractionalPartsWithIndices) {
        if (currentFractionalPart == fractionalPart) {
          order.back().push_back(index);
        } else {
          order.emplace_back(std::list<ClockVariables>{index});
          currentFractionalPart = fractionalPart;
        }
      }

      size = fractionalParts.size();
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
        result.order.pop_back();
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
     * @brief Remove \f$x_{N}\f$
     *
     * @pre \f$x_{N} = 0\f$
     */
    [[nodiscard]] FractionalOrder removeN() const {
      FractionalOrder result = *this;
      assert(result.order.front().back() + 1 == this->size);
      result.order.front().pop_back();
      result.size--;
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

    bool operator==(const FractionalOrder &another) const {
      return this->size == another.size && this->order == another.order;
    }

    std::ostream &print(std::ostream &os) const {
      auto it = order.begin();
      if (it->empty()) {
        os << "0 < ";
        it++;
      } else {
        os << "0 <= ";
      }
      for (; it != order.end(); it++) {
        os << "{";
        for (const auto var: *it) {
          os << "x" << int(var) << ", ";
        }
        os << "}";
      }

      return os;
    }
  };

  static inline std::ostream &operator<<(std::ostream &os, const learnta::FractionalOrder &order) {
    return order.print(os);
  }
}
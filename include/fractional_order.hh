/**
 * @author Masaki Waga
 * @date 2022/03/05.
 */

#pragma once

#include <deque>
#include <unordered_set>

#include <boost/functional/hash.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

#include "common_types.hh"

namespace learnta {
  /*!
   * @brief Order on the fractional part of the variables
   *
   * We implement this order by a list of sets of integers. For example, [{x1, x2}, {x3}, {x4}] represents 0 = x1 = x2 < x3 < x4.
   */
  class FractionalOrder {
  private:
    std::deque<std::deque<ClockVariables>> order;
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
      for (std::size_t i = 0; i < fractionalParts.size(); ++i) {
        fractionalPartsWithIndices.at(i) = std::make_pair(fractionalParts.at(i), i);
      }
      std::sort(fractionalPartsWithIndices.begin(), fractionalPartsWithIndices.end());
      double currentFractionalPart = 0;
      order.emplace_back();
      for (const auto&[fractionalPart, index]: fractionalPartsWithIndices) {
        if (currentFractionalPart == fractionalPart) {
          order.back().push_back(index);
        } else {
          order.emplace_back(std::deque<ClockVariables>{index});
          currentFractionalPart = fractionalPart;
        }
      }

      size = fractionalParts.size();
    }

    /*!
     * @brief Return the variable to elapse
     */
    [[nodiscard]] const std::deque<ClockVariables>& successorVariables() const {
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
     * @brief Make it to be the successor
     */
    void successorAssign() {
      if (order.front().empty()) {
        // If there is no variables equal to 0.
        std::swap(order.front(), order.back());
        order.pop_back();
      } else {
        // If there are some variables equal to 0.
        order.emplace_front();
      }
    }

    /*!
     * @brief Return the variable to backward-elapse
     */
    [[nodiscard]] std::deque<ClockVariables> predecessorVariables() const {
      if (order.empty()) {
        BOOST_LOG_TRIVIAL(error) << "Something wrong happened in the predecessorVariables. order is empty";
      }
      if (order.front().empty()) {
        if (order.size() <= 1) {
          BOOST_LOG_TRIVIAL(error) << "Something wrong happened in the predecessorVariables. No variable exists";
        }
        return *(std::next(order.begin()));
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
      assert(static_cast<std::size_t>(result.order.front().back() + 1) == this->size);
      assert(result.size > 0);
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

    [[nodiscard]] std::size_t hash_value() const {
      return boost::hash_value(std::make_pair(this->order, this->size));
    }
  };

  static inline std::ostream &operator<<(std::ostream &os, const learnta::FractionalOrder &order) {
    return order.print(os);
  }

  inline std::size_t hash_value(learnta::FractionalOrder const &order) {
    return order.hash_value();
  }
}
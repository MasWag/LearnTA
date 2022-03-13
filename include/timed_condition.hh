#pragma once

#include <utility>

#include "zone.hh"
#include "juxtaposed_zone.hh"

namespace learnta {
  /*!
    @brief A timed condition, a finite conjunction of inequalities of the form \f$\tau_{i} + \tau_{i + 1} \dots \tau_{j} \bowtie c\f$, where \f${\bowtie} \in \{>,\ge,\le,<\}\f$ and \f$c \in \mathbb{N} \f$.

    Let \f$x_0, x_1, \dots x_N\f$ be the variables in the zone. We use \f$x_i\f$ to represent \f$\mathbb{T}_{i,N} = \tau_{i} + \tau_{i+1} \dots \tau_{N}\f$. We note that the first row and column with index 0 of DBM are for the constant 0, and we have to shift the index appropriately.

    @note Policy: We wrap all the low-level DBM operations in this class.
   */
  class TimedCondition {
    /*!
      @brief A zone to represent the timing constraint
    */
  protected:
    Zone zone;

    explicit TimedCondition(Zone zone) : zone(std::move(zone)) {}

  public:
    TimedCondition() : zone(Zone::zero(2)) {}

    /*!
     * @brief Construct the empty timed condition, i.e. \f$\tau_0 = 0\f$.
     */
    static TimedCondition empty() {
      TimedCondition timedCondition;
      // The size of the DBM is 2 for \f$\tau_0\f$ and the special dimension for the constant zero.
      timedCondition.zone = Zone::zero(2);
      return timedCondition;
    }

    /*!
     * @brief Return the number of the variables in this timed condition
     */
    [[nodiscard]] size_t size() const {
      return this->zone.getNumOfVar();
    }

    /*!
      @brief Returns if this timed condition is simple
      
      @pre zone is canonical
    */
    [[nodiscard]] bool isSimple() const {
      for (int i = 0; i < zone.value.cols(); i++) {
        for (int j = i + 1; j < zone.value.cols(); j++) {
          // Note: zone.value(i, j) is not larger than zone.value(i, j)
          Bounds upperBound = zone.value(i, j); // i - j \le (c, s)
          Bounds lowerBound = zone.value(j, i); // j - i \le (c, s)
          if ((!isPoint(upperBound, lowerBound)) and (!isUnitOpen(upperBound, lowerBound))) {
            return false;
          }
        }
      }
      return true;
    }

    /*!
     * @brief Concatenate two timed conditions
     *
     * Let \f$N\f$ and \f$M\f$ be the dimensions of the concatenated timed conditions \f$\Lambda\f$ and \f$\Lambda'\f$. The resulting timed condition \f$\Lambda''\f$ satisfies the following.
     *
     * - The constraint on \f$\mathbb{T}_{i,j}\f$ in \f$\Lambda\f$ is the same as the constraint on \f$\mathbb{T}''_{i,j}\f$ in \f$\Lambda''\f$ if \f$ 0 \leq i \leq j < N\f$.
     * - The constraint on \f$\mathbb{T}'_{i,j}\f$ in \f$\Lambda'\f$ is the same as the constraint on \f$\mathbb{T}''_{i + N,j + N}\f$ in \f$\Lambda''\f$ if \f$ 0 < i \leq j \leq M\f$.
     * - The constraint on \f$\mathbb{T}''_{i,j}\f$ in \f$\Lambda''\f$ is the same as the constraint on \f$\mathbb{T}_{i,N} + \mathbb{T}'_{0, j - N}\f$ if \f$ 0 \leq i < N \leq j\f$.
     *
     * @post The dimension of the resulting timed conditions is the sum of the dimensions of the inputs - 1.
     */
    [[nodiscard]] TimedCondition operator+(const TimedCondition &another) const {
      TimedCondition result;
      const size_t N = this->size();
      const size_t M = another.size();
      result = TimedCondition();
      result.zone = Zone::top(N + M);
      // Copy \f$\mathbb{T}'\f$
      result.zone.value.block(N, N, M, M) = another.zone.value.block(1, 1, M, M);
      result.zone.value.block(0, N + 1, 1, M - 1) = another.zone.value.block(0, 1, 1, M - 1);
      result.zone.value.block(N + 1, 0, M - 1, 1) = another.zone.value.block(1, 0, M - 1, 1);
      // Copy \f$\mathbb{T}\f$
      result.zone.value.block(0, 0, N + 1, N + 1) = this->zone.value.block(0, 0, N + 1, N + 1);
      // Construct \f$\mathbb{T}''_{i, N + M}\f$ for each i <= N
      result.zone.value.block(0, 0, 1, N + 1).array() += another.zone.value(0, 1);
      result.zone.value.block(0, 0, N + 1, 1).array() += another.zone.value(1, 0);
      result.zone.canonize();

      return result;
    }

    /*!
     * @brief Juxtapose two timed conditions
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     */
    [[nodiscard]] JuxtaposedZone operator^(const TimedCondition &another) const {
      return JuxtaposedZone{this->zone, another.zone};
    }

    /*!
     * @brief Juxtapose two timed conditions renaming variable
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     */
    [[nodiscard]] JuxtaposedZone juxtaposeRight(const TimedCondition &right, std::size_t commonVariableSize) const {
      return JuxtaposedZone{this->zone, right.zone, commonVariableSize};
    }

    /*!
     * @brief Juxtapose two timed conditions renaming variable
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     */
    [[nodiscard]] JuxtaposedZone juxtaposeLeft(const TimedCondition &left, std::size_t commonVariableSize) const {
      return JuxtaposedZone{left.zone, this->zone, commonVariableSize};
    }

    /*!
     * @brief Returns the lower bound of \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$.
     */
    [[nodiscard]] Bounds getLowerBound(int i, int j) const {
      assert(0 <= i && i < this->size());
      assert(0 <= j && j < this->size());
      if (j == this->size() - 1) {
        return this->zone.value(0, i + 1);
      } else {
        return this->zone.value(j + 2, i + 1);
      }
    }

    /*!
     * @brief Returns the upper bound of \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$.
     */
    [[nodiscard]] Bounds getUpperBound(int i, int j) const {
      assert(0 <= i && i < this->size());
      assert(0 <= j && j < this->size());
      if (j == this->size() - 1) {
        return this->zone.value(i + 1, 0);
      } else {
        return this->zone.value(i + 1, j + 2);
      }
    }

    /*!
     * @brief Restrict the lower bound of \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$.
     *
     * @post zone is canonical
     */
    void restrictLowerBound(int i, int j, Bounds lowerBound) {
      assert(0 <= i && i < this->size());
      assert(0 <= j && j < this->size());
      if (j == this->size() - 1) {
        this->zone.value(0, i + 1) = lowerBound;
      } else {
        this->zone.value(j + 2, i + 1) = lowerBound;
      }
      this->zone.canonize();
    }

    /*!
     * @brief Restrict the upper bound of \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$.
     *
     * @post zone is canonical
     */
    void restrictUpperBound(int i, int j, Bounds upperBound) {
      assert(0 <= i && i < this->size());
      assert(0 <= j && j < this->size());
      if (j == this->size() - 1) {
        this->zone.value(i + 1, 0) = upperBound;
      } else {
        this->zone.value(i + 1, j + 2) = upperBound;
      }
      this->zone.canonize();
    }

    /*!
     * @brief Make it to be the convex hull of this timed condition and the given timed condition
     */
    void convexHullAssign(TimedCondition condition) {
      this->zone.value.array().max(condition.zone.value.array());
    }

    /*!
     * @brief Return the convex hull of this timed condition and the given timed condition
     */
    [[nodiscard]] TimedCondition convexHull(const TimedCondition &condition) const {
      Zone result = this->zone;
      result.value.array().max(condition.zone.value.array());
      return TimedCondition{result};
    }

    /*!
     * @brief Make a vector of simple timed conditions in this timed condition
     *
     * The construction is as follows.
     * - For each \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$, we restrict the constraints to be a point or a unit open interval.
     *   - This can be unnecessary. In that case, we remain the timed condition as it is.
     * - If the restricted timed condition is simple, we add it the resulting vector.
     * - Otherwise, we keep refining it.
     *
     * @pre zone is canonical
     */
    void enumerate(std::vector<TimedCondition> &simpleConditions) const {
      if (this->isSimple()) {
        simpleConditions = {*this};
        return;
      }
      std::vector<TimedCondition> currentConditions = {*this};
      for (int i = 0; i < this->size(); i++) {
        for (int j = i; j < this->size(); j++) {
          std::vector<TimedCondition> nextConditions;
          for (const TimedCondition &timedCondition: currentConditions) {
            if (timedCondition.isSimple()) {
              simpleConditions.push_back(timedCondition);
              continue;
            }
            auto lowerBound = timedCondition.getLowerBound(i, j);
            const auto upperBound = timedCondition.getUpperBound(i, j);
            if (isPoint(upperBound, lowerBound) or isUnitOpen(upperBound, lowerBound)) {
              nextConditions.push_back(timedCondition);
            } else {
              while (-lowerBound <= upperBound) {
                auto currentTimedCondition = timedCondition;
                if (lowerBound.second) {
                  currentTimedCondition.restrictLowerBound(i, j, lowerBound);
                  currentTimedCondition.restrictUpperBound(i, j, -lowerBound);
                  lowerBound.second = false;
                } else {
                  currentTimedCondition.restrictLowerBound(i, j, lowerBound);
                  currentTimedCondition.restrictUpperBound(i, j, {-lowerBound.first + 1, false});
                  lowerBound = {lowerBound.first - 1, true};
                }
                if (currentTimedCondition.isSimple()) {
                  simpleConditions.push_back(currentTimedCondition);
                } else {
                  nextConditions.push_back(currentTimedCondition);
                }
              }
            }
          }
          currentConditions = std::move(nextConditions);
          if (currentConditions.empty()) {
            return;
          }
        }
      }
    }

    /*!
     * @brief Make a continuous successor by elapsing variables
     */
    [[nodiscard]] TimedCondition successor(const std::list<ClockVariables> &variables) const {
      Zone result = this->zone;

      for (const auto i: variables) {
        // Bound of \f$\mathbb{T}_{i,N}
        Bounds &upperBound = result.value(i + 1, 0);
        Bounds &lowerBound = result.value(0, i + 1);
        if (isPoint(upperBound, lowerBound)) {
          upperBound.first++;
          upperBound.second = false;
          lowerBound.second = false;
        } else {
          lowerBound.first--;
          lowerBound.second = true;
          upperBound.second = true;
        }
      }

      return TimedCondition(result);
    }

    /*!
     * @brief Make a continuous predecessor by backward-elapsing variables
     */
    [[nodiscard]] TimedCondition predecessor(const std::list<ClockVariables> &variables) const {
      Zone result = this->zone;

      for (const auto i: variables) {
        // Bound of \f$\mathbb{T}_{i,N}
        Bounds &upperBound = result.value(i + 1, 0);
        Bounds &lowerBound = result.value(0, i + 1);
        if (isPoint(upperBound, lowerBound)) {
          lowerBound.first++;
          upperBound.second = false;
          lowerBound.second = false;
        } else {
          upperBound.first--;
          lowerBound.second = true;
          upperBound.second = true;
        }
      }

      return TimedCondition(result);
    }

    /*!
     * @brief Add another variable \f$x_{n+1}\f$ such that \f$x_n = x_{n+1}\f$.
     */
    [[nodiscard]] TimedCondition extendN() const {
      TimedCondition result = *this;
      const auto N = result.zone.value.cols();
      result.zone.value.conservativeResize(N + 1, N + 1);
      result.zone.value.col(N) = result.zone.value.col(0);
      result.zone.value.row(N) = result.zone.value.row(0);
      // Add \f$x_n = x_{n+1}\f$
      result.zone.value(N, 0) = {0, true};
      result.zone.value(0, N) = {0, true};

      return result;
    }

    /*!
     * @brief Return if there is \f$\mathbb{T}_{i,N} = c\f$.
     *
     * @pre the timed condition is simple
     */
    [[nodiscard]] bool hasEqualityN() const {
      // By simplicity of the timed condition, we can check only the one side
      return this->zone.value.array().col(0).unaryExpr([](const Bounds &bound) {
        return bound.second;
      }).any();
    }

    /*!
     * @brief Rename each variable \f$x_i\f$ to \f$x_{i+1}\f$ and add \f$x_0\f$ such that \f$x_0 = x_1\f$.
     */
    [[nodiscard]] TimedCondition extendZero() const {
      const auto N = this->zone.value.cols();
      auto result = Zone::top(N + 1);
      // Fill the constraints with shift
      result.value.block(2, 2, N - 1, N - 1) = this->zone.value.block(1, 1, N - 1, N - 1);
      result.value.block(0, 2, 1, N - 1) = this->zone.value.block(0, 1, 1, N - 1);
      result.value.block(2, 0, N - 1, 1) = this->zone.value.block(1, 0, N - 1, 1);
      // Copy the constraints of \f$x_1\f$ to \f$x_{0}\f$.
      result.value.col(1) = result.value.col(2);
      result.value.row(1) = result.value.row(2);
      // Add \f$x_0 = x_1\f$
      result.value(1, 2) = {0, true};
      result.value(2, 1) = {0, true};

      return TimedCondition(result);
    }

    /*!
     * @brief Returns the set of variables strictly constrained compared with the original condition.
     *
     * @pre this condition and original condition should have the same variable space.
     */
    [[nodiscard]] std::vector<std::size_t> getStrictlyConstrainedVariables(const TimedCondition &originalCondition,
                                                                           const size_t examinedVariableSize) const {
      std::vector<std::size_t> result;
      for (int i = 1; i <= examinedVariableSize; ++i) {
        if (this->zone.value.col(i) != originalCondition.zone.value.col(i) ||
            this->zone.value.row(i) != originalCondition.zone.value.row(i)) {
          result.push_back(i);
        }
      }
      return result;
    }

    bool operator==(const TimedCondition &condition) const {
      return this->zone == condition.zone;
    }
  };
}


namespace learnta {
  static inline std::ostream &print(std::ostream &os, const learnta::TimedCondition &cond) {
    for (int i = 0; i < cond.size(); ++i) {
      for (int j = i; j < cond.size(); ++j) {
        const auto upperBound = cond.getUpperBound(i, j);
        const auto lowerBound = cond.getLowerBound(i, j);
        os << -lowerBound.first << (lowerBound.second ? " <= " : " < ")
           << "T_{" << i << ", " << j << "} "
           << (upperBound.second ? " <= " : " < ") << upperBound.first;
        if (i != cond.size() - 1 || j != cond.size() - 1) {
          os << " && ";
        }
      }
    }
    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const learnta::TimedCondition &b) {
    return learnta::print(os, b);
  }
}

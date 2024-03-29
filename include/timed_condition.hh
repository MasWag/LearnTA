#pragma once

#include <utility>
#include <deque>
#include <iostream>

#include "zone.hh"
#include "juxtaposed_zone.hh"
#include "timed_automaton.hh"

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

    explicit TimedCondition(Zone &&zone) : zone(zone) {}

  public:
    TimedCondition() : zone(Zone::zero(2)) {}

    /*!
     * @brief Construct a timed condition from concrete values of T_{i,j}.
     * The generated timed condition is the simple timed condition containing the given concrete valuation.
     *
     * @param [in] accumulatedDuration a vector representing \f$\mathbb{T}_{i,N}\f$, where \f$N\f$ is the length.
     */
    explicit TimedCondition(const std::vector<double> &accumulatedDuration) {
      this->zone = Zone::top(accumulatedDuration.size() + 1);
      for (std::size_t i = 0; i < accumulatedDuration.size(); ++i) {
        for (std::size_t j = i; j < accumulatedDuration.size(); ++j) {
          // T_{i, j} = accumulatedDuration.at(i) - accumulatedDuration.at(j + 1)
          const auto concreteDifference = accumulatedDuration.at(i) -
                                          ((j + 1 < accumulatedDuration.size()) ? accumulatedDuration.at(j + 1) : 0);
          if (std::floor(concreteDifference) == concreteDifference) {
            this->restrictUpperBound(i, j, Bounds{concreteDifference, true}, true);
            this->restrictLowerBound(i, j, Bounds{-concreteDifference, true}, true);
          } else {
            this->restrictUpperBound(i, j, Bounds{std::floor(concreteDifference) + 1, false}, true);
            this->restrictLowerBound(i, j, Bounds{-std::floor(concreteDifference), false}, true);
          }
        }
      }
    }

    /*!
     * @brief Construct a timed condition from concrete values of T_{i,j}.
     * The generated timed condition only contains the given concrete valuation.
     *
     * @param [in] accumulatedDuration a vector representing \f$\mathbb{T}_{i,N}\f$, where \f$N\f$ is the length.
     */
    static TimedCondition makeExact(const std::vector<double> &accumulatedDuration) {
      TimedCondition result{Zone::top(accumulatedDuration.size() + 1)};
      for (std::size_t i = 0; i < accumulatedDuration.size(); ++i) {
        for (std::size_t j = i; j < accumulatedDuration.size(); ++j) {
          // T_{i, j} = accumulatedDuration.at(i) - accumulatedDuration.at(j + 1)
          const auto concreteDifference = accumulatedDuration.at(i) -
                                          ((j + 1 < accumulatedDuration.size()) ? accumulatedDuration.at(j + 1) : 0);
          result.restrictUpperBound(i, j, Bounds{concreteDifference, true}, true);
          result.restrictLowerBound(i, j, Bounds{-concreteDifference, true}, true);
        }
      }

      return result;
    }

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
          if ((!learnta::isPoint(upperBound, lowerBound)) and (!isUnitOpen(upperBound, lowerBound))) {
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
     * - If \f$ 0 \leq i \leq j < N\f$, the constraint on \f$\mathbb{T}''_{i,j}\f$ in \f$\Lambda''\f$ is the same as the constraint on \f$\mathbb{T}_{i,j}\f$ in \f$\Lambda\f$.
     * - If \f$ N < i \leq j \leq N + M\f$, the constraint on \f$\mathbb{T}''_{i, j}\f$ in \f$\Lambda''\f$ is the same as the constraint on \f$\mathbb{T}'_{i - N, j - N}\f$ in \f$\Lambda'\f$.
     * - If \f$ 0 \leq i \leq N \leq j\f$, the constraint on \f$\mathbb{T}''_{i,j}\f$ in \f$\Lambda''\f$ is the same as the constraint on \f$\mathbb{T}_{i, N} + \mathbb{T}'_{0, j - N}\f$.
     *
     *
     *  Let \f$A, B, C\f$ be the DBM representing \f$\Lambda, \Lambda', \Lambda''\f$, respectively. To construct \f$C\f$ from \f$A\f$ and \f$B\f$, what we have to do is as follows.
     *
     *  - If \f$ 0 \leq i \leq j < N\f$, the constraint on \f$\mathbb{T}''_{i,j}\f$ in \f$\Lambda''\f$ is the same as the constraint on \f$\mathbb{T}_{i,j}\f$ in \f$\Lambda\f$.
     *      - Copy \f$A_{(1, 1), (N, N)}\f$ to \f$C_{(1, 1), (N, N)}\f$.
     *  - If \f$ N < i \leq j \leq N + M\f$, the constraint on \f$\mathbb{T}''_{i, j}\f$ in \f$\Lambda''\f$ is the same as the constraint on \f$\mathbb{T}'_{i - N, j - N}\f$ in \f$\Lambda'\f$.
     *      - Copy \f$B_{(2, 2), (M - 1, M - 1)}\f$ to \f$C_{(N + 1, N + 1), (M - 1, M - 1)}\f$.
     *      - Copy \f$B_{(2, 0), (M - 1, 1)}\f$ to \f$C_{(N + 1, 0), (M - 1, 1)}\f$.
     *      - Copy \f$B_{(0, 2), (1, M - 1)}\f$ to \f$C_{(0, N + 1), (1, M - 1)}\f$.
     *  - If \f$ 0 \leq i \leq N \leq j\f$, the constraint on \f$\mathbb{T}''_{i,j}\f$ in \f$\Lambda''\f$ is the same as the constraint on \f$\mathbb{T}_{i, N} + \mathbb{T}'_{0, j - N}\f$.
     *      - Copy \f$A_{(1, 0), (N, 1)}\f$ to \f$C_{(1, i), (N, 1)}\f$ for each \f$i \in \{0, N, N + 1, \dots, N + M - 1\}\f$.
     *      - Copy \f$A_{(0, 1), (1, N)}\f$ to \f$C_{(i, 1), (1, N)}\f$ for each \f$i \in \{0, N, N + 1, \dots, N + M - 1\}\f$.
     *      - Add  \f$B_{(2, 1), (M - 1, 1)}\f$ to \f$C_{(N + 1, i), (M - 1, 1)}\f$ for each \f$i \in \{1, \dots, N\}\f$.
     *      - Add  \f$B_{(1, 2), (1, M - 1)}\f$ to \f$C_{(i, N + 1), (1, M - 1)}\f$ for each \f$i \in \{1, \dots, N\}\f$.
     *      - Add  \f$B_{(1, 0)}\f$ to \f$C_{(1, 0), (N, 1)}\f$.
     *      - Add  \f$B_{(0, 1)}\f$ to \f$C_{(0, 1), (1, N)}\f$.
     *
     *  By combining the above, what we do is as follows.
     *  - Copy \f$A_{(0, 0), (N + 1, N + 1)}\f$ to \f$C_{(0, 0), (N + 1, N + 1)}\f$.
     *  - Copy \f$A_{(1, 0), (N, 1)}\f$ to \f$C_{(1, i), (N, 1)}\f$ for each \f$i \in \{N, N + 1, \dots, N + M - 1\}\f$.
     *  - Copy \f$A_{(0, 1), (1, N)}\f$ to \f$C_{(i, 1), (1, N)}\f$ for each \f$i \in \{N, N + 1, \dots, N + M - 1\}\f$.
     *  - Copy \f$B_{(2, 2), (M - 1, M - 1)}\f$ to \f$C_{(N + 2, N + 2), (M - 1, M - 1)}\f$.
     *  - Copy \f$B_{(2, 0), (M - 1, 1)}\f$ to \f$C_{(N + 1, 0), (M - 1, 1)}\f$.
     *  - Copy \f$B_{(0, 2), (1, M - 1)}\f$ to \f$C_{(0, N + 1), (1, M - 1)}\f$.
     *  - Add  \f$B_{(2, 1), (M - 1, 1)}\f$ to \f$C_{(N + 1, i), (M - 1, 1)}\f$ for each \f$i \in \{1, \dots, N\}\f$.
     *  - Add  \f$B_{(1, 2), (1, M - 1)}\f$ to \f$C_{(i, N + 1), (1, M - 1)}\f$ for each \f$i \in \{1, \dots, N\}\f$.
     *  - Add  \f$B_{(1, 0)}\f$ to \f$C_{(1, 0), (N, 1)}\f$.
     *  - Add  \f$B_{(0, 1)}\f$ to \f$C_{(0, 1), (1, N)}\f$.
     *
     * @post The dimension of the resulting timed conditions is the sum of the dimensions of the inputs - 1.
     */
    [[nodiscard]] TimedCondition operator+(const TimedCondition &another) const {
      const size_t N = this->size();
      const size_t M = another.size();
      Zone result = Zone::top(N + M);
      // Copy \f$A_{(0, 0), (N + 1, N + 1)}\f$ to \f$C_{(0, 0), (N + 1, N + 1)}\f$.
      result.value.block(0, 0, N + 1, N + 1) = this->zone.value;
      for (std::size_t i = N + 1; i < N + M; ++i) {
        // Copy \f$A_{(1, 0), (N, 1)}\f$ to \f$C_{(1, i), (N, 1)}\f$ for each \f$i \in \{N, N + 1, \dots, N + M - 1\}\f$.
        result.value.block(1, i, N, 1) = this->zone.value.block(1, 0, N, 1);
        // Copy \f$A_{(0, 1), (1, N)}\f$ to \f$C_{(i, 1), (1, N)}\f$ for each \f$i \in \{N, N + 1, \dots, N + M - 1\}\f$.
        result.value.block(i, 1, 1, N) = this->zone.value.block(0, 1, 1, N);
      }
      if (M >= 2) {
        // Copy \f$B_{(2, 2), (M - 1, M - 1)}\f$ to \f$C_{(N + 1, N + 1), (M - 1, M - 1)}\f$.
        result.value.block(N + 1, N + 1, M - 1, M - 1) = another.zone.value.block(2, 2, M - 1, M - 1);
        // Copy \f$B_{(2, 0), (M - 1, 1)}\f$ to \f$C_{(N + 1, 0), (M - 1, 1)}\f$.
        result.value.block(N + 1, 0, M - 1, 1) = another.zone.value.block(2, 0, M - 1, 1);
        // Copy \f$B_{(0, 2), (1, M - 1)}\f$ to \f$C_{(0, N + 1), (1, M - 1)}\f$.
        result.value.block(0, N + 1, 1, M - 1) = another.zone.value.block(0, 2, 1, M - 1);
      }
      for (std::size_t i = 1; i <= N; ++i) {
        // Add  \f$B_{(2, 1), (M - 1, 1)}\f$ to \f$C_{(N + 1, i), (M - 1, 1)}\f$ for each \f$i \in \{1, \dots, N\}\f$.
        result.value.block(N + 1, i, M - 1, 1).array() += another.zone.value.block(2, 1, M - 1, 1).array();
        // Add  \f$B_{(1, 2), (1, M - 1)}\f$ to \f$C_{(i, N + 1), (1, M - 1)}\f$ for each \f$i \in \{1, \dots, N\}\f$.
        result.value.block(i, N + 1, 1, M - 1).array() += another.zone.value.block(1, 2, 1, M - 1).array();
      }
      // Add  \f$B_{(1, 0)}\f$ to \f$C_{(1, 0), (N, 1)}\f$.
      result.value.block(1, 0, N, 1).array() += another.zone.value(1, 0);
      // Add  \f$B_{(0, 1)}\f$ to \f$C_{(0, 1), (1, N)}\f$.
      result.value.block(0, 1, 1, N).array() += another.zone.value(0, 1);

      result.canonize();

      return TimedCondition{std::move(result)};
    }

    /*!
     * @brief Juxtapose two timed conditions
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     * @post The resulting JuxtaposedZone is canonical
     */
    [[nodiscard]] JuxtaposedZone operator^(const TimedCondition &another) const {
      return JuxtaposedZone{this->zone, another.zone};
    }

    /*!
     * @brief Juxtapose two timed conditions renaming variable
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     */
    [[nodiscard]] JuxtaposedZone juxtaposeRight(const TimedCondition &right, Eigen::Index commonVariableSize) const {
      return JuxtaposedZone{this->zone, right.zone, commonVariableSize};
    }

    /*!
     * @brief Juxtapose two timed conditions renaming variable
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     */
    [[nodiscard]] JuxtaposedZone juxtaposeLeft(const TimedCondition &left, Eigen::Index commonVariableSize) const {
      return JuxtaposedZone{left.zone, this->zone, commonVariableSize};
    }

    /*!
     * @brief Returns the lower bound of \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$.
     */
    [[nodiscard]] Bounds getLowerBound(std::size_t i, std::size_t j) const {
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
    [[nodiscard]] Bounds getUpperBound(std::size_t i, std::size_t j) const {
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
    void restrictLowerBound(std::size_t i, std::size_t j, Bounds lowerBound, bool force = false) {
      assert(0 <= i && i < this->size());
      assert(0 <= j && j < this->size());
      if (j == this->size() - 1) {
        this->zone.value(0, i + 1) = force ? lowerBound : std::min(lowerBound, this->zone.value(0, i + 1));
      } else {
        this->zone.value(j + 2, i + 1) = force ? lowerBound : std::min(lowerBound, this->zone.value(j + 2, i + 1));
      }
      this->zone.canonize();
    }

    /*!
     * @brief Restrict the upper bound of \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$.
     *
     * @post zone is canonical
     */
    void restrictUpperBound(std::size_t i, std::size_t j, Bounds upperBound, bool force = false) {
      assert(0 <= i && i < this->size());
      assert(0 <= j && j < this->size());
      if (j == this->size() - 1) {
        this->zone.value(i + 1, 0) = force ? upperBound : std::min(upperBound, this->zone.value(i + 1, 0));
      } else {
        this->zone.value(i + 1, j + 2) = force ? upperBound : std::min(upperBound, this->zone.value(i + 1, j + 2));
      }
      this->zone.canonize();
    }

    /*!
     * @brief Make it to be the convex hull of this timed condition and the given timed condition
     */
    void convexHullAssign(const TimedCondition &condition) {
      this->zone.value = this->zone.value.cwiseMax(condition.zone.value);
    }

    /*!
     * @brief Return the convex hull of this timed condition and the given timed condition
     */
    [[nodiscard]] TimedCondition convexHull(const TimedCondition &condition) const {
      return TimedCondition{Zone{this->zone.value.cwiseMax(condition.zone.value)}};
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
      for (std::size_t i = 0; i < this->size(); i++) {
        for (std::size_t j = i; j < this->size(); j++) {
          std::vector<TimedCondition> nextConditions;
          for (const TimedCondition &timedCondition: currentConditions) {
            if (timedCondition.isSimple()) {
              simpleConditions.push_back(timedCondition);
              continue;
            }
            auto lowerBound = timedCondition.getLowerBound(i, j);
            const auto upperBound = timedCondition.getUpperBound(i, j);
            if (learnta::isPoint(upperBound, lowerBound) or isUnitOpen(upperBound, lowerBound)) {
              nextConditions.push_back(timedCondition);
            } else {
              Bounds currentUpperBound = lowerBound.second ? -lowerBound : std::make_pair(-lowerBound.first + 1, false);
              while (currentUpperBound <= upperBound) {
                auto currentTimedCondition = timedCondition;
                currentTimedCondition.restrictLowerBound(i, j, lowerBound, false);
                currentTimedCondition.restrictUpperBound(i, j, currentUpperBound, false);
                if (lowerBound.second) {
                  currentUpperBound = {-lowerBound.first + 1, false};
                  lowerBound.second = false;
                } else {
                  currentUpperBound = std::make_pair(-lowerBound.first + 1, true);
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
    [[nodiscard]] std::vector<TimedCondition> enumerate() const {
      std::vector<TimedCondition> simpleConditions;
      this->enumerate(simpleConditions);

      return simpleConditions;
    }

    /*!
     * @brief Make a continuous successor by elapsing variables
     */
    [[nodiscard]] TimedCondition successor(const std::deque<ClockVariables> &variables) const {
      Zone result = this->zone;

      for (const auto i: variables) {
        // Bound of \f$\mathbb{T}_{i,N}
        Bounds &upperBound = result.value(i + 1, 0);
        Bounds &lowerBound = result.value(0, i + 1);
        if (lowerBound.second) {
          upperBound.first++;
          upperBound.second = false;
          lowerBound.second = false;
        } else {
          lowerBound.first--;
          lowerBound.second = true;
          upperBound.second = true;
        }
      }

      return TimedCondition(std::move(result));
    }

    /*!
     * @brief Make a continuous successor by elapsing variables
     */
    void successorAssign(const std::deque<ClockVariables> &variables) {
      for (const auto i: variables) {
        // Bound of \f$\mathbb{T}_{i,N}
        Bounds &upperBound = this->zone.value(i + 1, 0);
        Bounds &lowerBound = this->zone.value(0, i + 1);
        if (lowerBound.second) {
          upperBound.first++;
          upperBound.second = false;
          lowerBound.second = false;
        } else {
          lowerBound.first--;
          lowerBound.second = true;
          upperBound.second = true;
        }
      }
    }

    /*!
     * @brief Remove the equality upper bound
     */
    void removeEqualityUpperBoundAssign() {

      for (std::size_t i = 0; i < this->zone.getNumOfVar(); i++) {
        // Bound of \f$\mathbb{T}_{i,N}
        Bounds &upperBound = this->zone.value(i + 1, 0);
        if (upperBound.second) {
          upperBound = Bounds{std::numeric_limits<double>::max(), false};
        }
      }
    }

    /*!
     * @brief Remove the upper bounds
     */
    void removeUpperBoundAssign() {
      for (std::size_t i = 0; i < this->zone.getNumOfVar(); i++) {
        // Bound of \f$\mathbb{T}_{i,N}
        this->zone.value(i + 1, 0) = Bounds{std::numeric_limits<double>::max(), false};
      }
    }

    /*!
     * @brief Make a continuous predecessor by backward-elapsing variables
     */
    [[nodiscard]] TimedCondition predecessor(const std::deque<ClockVariables> &variables) const {
      Zone result = this->zone;

      for (const auto i: variables) {
        // Bound of \f$\mathbb{T}_{0, i} = \mathbb{T}_{0, N} - \mathbb{T}_{i + 1, N}\f$
        Bounds &upperBound = result.value(1, (i + 2) % result.value.cols());
        Bounds &lowerBound = result.value((i + 2) % result.value.cols(), 1);
        if (learnta::isPoint(upperBound, lowerBound)) {
          upperBound.first++;
          upperBound.second = false;
          lowerBound.second = false;
        } else {
          lowerBound.first--;
          lowerBound.second = true;
          upperBound.second = true;
        }
      }

      return TimedCondition(std::move(result));
    }

    /*!
     * @brief Make a continuous prefix
     */
    [[nodiscard]] TimedCondition prefix(const std::deque<ClockVariables> &variables) const {
      Zone result = this->zone;

      for (const auto i: variables) {
        // Bound of \f$\mathbb{T}_{i, N}
        Bounds &upperBound = result.value(i + 1, 0);
        Bounds &lowerBound = result.value(0, i + 1);
        if (learnta::isPoint(upperBound, lowerBound)) {
          upperBound.second = false;
          lowerBound.first++;
          lowerBound.second = false;
        } else {
          lowerBound.second = true;
          upperBound.first--;
          upperBound.second = true;
        }
      }

      return TimedCondition(std::move(result));
    }

    /*!
     * @brief Make a continuous suffix
     */
    [[nodiscard]] TimedCondition suffix(const std::deque<ClockVariables> &variables) const {
      Zone result = this->zone;

      for (const auto i: variables) {
        // Bound of \f$\mathbb{T}_{0, i}
        Bounds &upperBound = (i == this->size() - 1) ? result.value(1, 0) : result.value(1, i + 2);
        Bounds &lowerBound = (i == this->size() - 1) ? result.value(0, 1) : result.value(i + 2, 1);
        if (learnta::isPoint(upperBound, lowerBound)) {
          upperBound.second = false;
          lowerBound.first++;
          lowerBound.second = false;
        } else {
          lowerBound.second = true;
          upperBound.first--;
          upperBound.second = true;
        }
      }

      return TimedCondition(std::move(result));
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
     * @brief Remove \f$x_{N}\f$
     */
    [[nodiscard]] TimedCondition removeN() const {
      TimedCondition result = *this;
      const auto N = result.zone.value.cols();
      result.zone.value.conservativeResize(N - 1, N - 1);

      return result;
    }

    /*!
     * @brief Remove \f$x_{0}\f$
     * TODO: Implement it
     */
    [[nodiscard]] TimedCondition removeZero() const {
      TimedCondition result = *this;
      const auto N = result.zone.value.cols();
      result.zone.value.conservativeResize(N - 1, N - 1);

      return result;
    }

    /*!
     * @brief Return if there is \f$\mathbb{T}_{i,N} = c\f$.
     *
     * @pre the timed condition is simple
     */
    [[nodiscard]] bool hasEqualityN() const {
      // By simplicity of the timed condition, we can check only the one side
      return this->zone.value.col(0).tail(this->size()).unaryExpr([](const Bounds &bound) {
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

      return TimedCondition(std::move(result));
    }

    /*!
     * @brief Returns the set of variables strictly constrained compared with the original condition.
     *
     * @pre this condition and original condition should have the same variable space.
     */
    [[nodiscard]] std::vector<std::size_t> getStrictlyConstrainedVariables(const TimedCondition &originalCondition,
                                                                           const size_t examinedVariableSize) const {
      std::vector<std::size_t> result;
      for (std::size_t i = 1; i <= examinedVariableSize; ++i) {
        if (this->zone.value.col(i) != originalCondition.zone.value.col(i) ||
            this->zone.value.row(i) != originalCondition.zone.value.row(i)) {
          result.push_back(i - 1);
        }
      }
      return result;
    }

    bool operator==(const TimedCondition &condition) const {
      return this->size() == condition.size() && this->zone.strictEqual(condition.zone);
    }

    bool operator!=(const TimedCondition &condition) const {
      return this->size() != condition.size() || !this->zone.strictEqual(condition.zone);
    }

    /*!
     * @breif Construct a guard over \f${x_0, x_1,\dots,x_N}\f$ such that \f$x_i = \mathbb{T}_{i,N}\f$.
     */
    [[nodiscard]] std::vector<Constraint> toGuard() const {
#ifdef DEBUG
      BOOST_LOG_TRIVIAL(trace) << "Constraint:" << *this;
#endif
      std::vector<Constraint> result;
      const auto N = this->size();
      result.reserve(N * 2);
      for (std::size_t i = 0; i < this->size(); ++i) {
        const auto lowerBound = this->getLowerBound(i, N - 1);
        const auto upperBound = this->getUpperBound(i, N - 1);
        if (lowerBound.first != std::numeric_limits<double>::max() && lowerBound != Bounds{0, true}) {
          if (lowerBound.second) {
            result.push_back(ConstraintMaker(i) >= -int(lowerBound.first));
          } else {
            result.push_back(ConstraintMaker(i) > -int(lowerBound.first));
          }
        }
        if (upperBound.first != std::numeric_limits<double>::max()) {
          if (upperBound.second) {
            result.push_back(ConstraintMaker(i) <= int(upperBound.first));
          } else {
            result.push_back(ConstraintMaker(i) < int(upperBound.first));
          }
        }
      }

#ifdef DEBUG
      BOOST_LOG_TRIVIAL(trace) << "Guard: " << result;
#endif
      return result;
    }

    /*!
     * @brief Return if this timed condition has a (continuous) prefix
     */
    [[nodiscard]] bool hasPrefix() const {
      const auto N = zone.value.cols() - 1;

      return !(this->getUpperBound(N - 1, N - 1) == Bounds{0, true} &&
               this->getLowerBound(N - 1, N - 1) == Bounds{0, true});
    }

    /*!
     * @brief Return if this timed condition has a (continuous) suffix
     */
    [[nodiscard]] bool hasSuffix() const {

      return !(this->getUpperBound(0, 0) == Bounds{0, true} &&
               this->getLowerBound(0, 0) == Bounds{0, true});
    }

    /*!
     * @brief Return if this timed condition includes the given timed condition
     */
    [[nodiscard]] bool includes(const TimedCondition &condition) const {
      return this->zone.includes(condition.zone);
    }

    /*!
     * @brief Returns the intersection of two timed conditions
     */
    TimedCondition operator&&(const TimedCondition &another) const {
      return TimedCondition{this->zone && another.zone};
    }

    /*!
     * @brief Returns if the timed condition is satisfiable
     */
    [[nodiscard]] bool isSatisfiableNoCanonize() const {
      return this->zone.isSatisfiableNoCanonize();
    }

    /*!
     * @brief Returns if the timed condition is satisfiable
     */
    [[nodiscard]] explicit operator bool() {
      return this->zone.isSatisfiable();
    }

    [[nodiscard]] TimedCondition applyResets(const TATransition::Resets &resets) const {
      auto newCondition = *this;
      for (const auto &[updatedVariable, assignedValue]: resets) {
        // We do not support renaming here.
        assert(assignedValue.index() == 0);
        // Unconstrain the assigned variable
        newCondition.zone.unconstrain(updatedVariable);
        if (std::get<double>(assignedValue) == std::floor(std::get<double>(assignedValue))) {
          newCondition.restrictLowerBound(updatedVariable, newCondition.size() - 1,
                                          Bounds{-std::get<double>(assignedValue), true}, true);
          newCondition.restrictUpperBound(updatedVariable, newCondition.size() - 1,
                                          Bounds{std::get<double>(assignedValue), true}, true);
        } else {
          newCondition.restrictLowerBound(updatedVariable, newCondition.size() - 1,
                                          Bounds{-std::floor(std::get<double>(assignedValue)), false}, true);
          newCondition.restrictUpperBound(updatedVariable, newCondition.size() - 1,
                                          Bounds{std::ceil(std::get<double>(assignedValue)), false}, true);
        }
      }

      return newCondition;
    }

    /*!
     * @brief Return the timed condition after applying the given reset
     *
     * @param resets The applied reset
     * @param targetClockSize The number of the clock variables in the target language
     *
     * @post There is a clock valuation in this condition such that its value after reset is in the resulting condition.
     */
    [[nodiscard]] TimedCondition applyResets(const TATransition::Resets &resets,
                                             const std::size_t targetClockSize) const {
      TimedCondition newCondition {Zone::top(targetClockSize + 1)};
      std::vector<std::pair<std::size_t, std::size_t>> renaming;
      for (const auto &[updatedVariable, assignedValue]: resets) {
        if (updatedVariable >= targetClockSize) {
          continue;
        }
        if(assignedValue.index() == 0) {
          // Apply non-renaming resets
          if (std::get<double>(assignedValue) == std::floor(std::get<double>(assignedValue))) {
            newCondition.restrictLowerBound(updatedVariable, newCondition.size() - 1,
                                            Bounds{-std::get<double>(assignedValue), true}, true);
            newCondition.restrictUpperBound(updatedVariable, newCondition.size() - 1,
                                            Bounds{std::get<double>(assignedValue), true}, true);
          } else {
            newCondition.restrictLowerBound(updatedVariable, newCondition.size() - 1,
                                            Bounds{-std::floor(std::get<double>(assignedValue)), false}, true);
            newCondition.restrictUpperBound(updatedVariable, newCondition.size() - 1,
                                            Bounds{std::ceil(std::get<double>(assignedValue)), false}, true);
          }
          for (const auto &[updatedVariable2, assignedValue2]: resets) {
            if (updatedVariable2 < updatedVariable && assignedValue2.index() == 0) {
              double difference = std::get<double>(assignedValue2) - std::get<double>(assignedValue);
              // Apply non-renaming resets
              if (difference == std::floor(difference)) {
                newCondition.restrictLowerBound(updatedVariable2, updatedVariable - 1,
                                                Bounds{-difference, true}, true);
                newCondition.restrictUpperBound(updatedVariable2, updatedVariable - 1,
                                                Bounds{difference, true}, true);
              } else {
                newCondition.restrictLowerBound(updatedVariable2, updatedVariable - 1,
                                                Bounds{-std::floor(difference), false}, true);
                newCondition.restrictUpperBound(updatedVariable2, updatedVariable - 1,
                                                Bounds{std::ceil(difference), false}, true);
              }
            }
          }
        } else {
          // add to renaming
          renaming.emplace_back(std::get<ClockVariables>(assignedValue), updatedVariable);
        }
      }
      // add implicit renaming
      for (ClockVariables clock = 0; clock < targetClockSize; ++clock) {
        auto it = std::find_if(resets.begin(), resets.end(), [&clock] (const auto &reset) {
          return reset.first == clock;
        });
        if (it == resets.end()) {
          renaming.emplace_back(clock, clock);
        }
      }
      auto juxtaposed = *this ^ newCondition;
      juxtaposed.addRenaming(renaming);

      return TimedCondition{juxtaposed.getRight()};
    }

    /*!
     * @brief Check if T_{i, |T|} is a point
     */
    [[nodiscard]] bool isPoint(std::size_t i) const {
      return learnta::isPoint(this->getUpperBound(i, this->size() - 1), this->getLowerBound(i, this->size() - 1));
    }

    [[nodiscard]] std::size_t hash_value() const {
      return learnta::hash_value(this->zone);
    }

    std::ostream &print(std::ostream &os) const {
      for (std::size_t i = 0; i < this->size(); ++i) {
        for (std::size_t j = i; j < this->size(); ++j) {
          const auto upperBound = this->getUpperBound(i, j);
          const auto lowerBound = this->getLowerBound(i, j);
          os << -lowerBound.first << (lowerBound.second ? " <= " : " < ")
             << "T_{" << i << ", " << j << "} "
             << (upperBound.second ? " <= " : " < ") << upperBound.first;
          if (i != this->size() - 1 || j != this->size() - 1) {
            os << " && ";
          }
        }
      }
      return os;
    }

    friend class NeighborConditions;
  };
}


namespace learnta {
  static inline std::ostream &print(std::ostream &os, const learnta::TimedCondition &cond) {
    for (std::size_t i = 0; i < cond.size(); ++i) {
      for (std::size_t j = i; j < cond.size(); ++j) {
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

  inline std::size_t hash_value(learnta::TimedCondition const &timedCondition) {
    return timedCondition.hash_value();
  }
}

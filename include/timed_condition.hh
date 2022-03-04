#include "zone.hh"

namespace learnta {
  /*!
    @brief A timed condition, a finite conjunction of inequalities of the form \f$\tau_{i} + \tau_{i + 1} \dots \tau_{j} \bowtie c\f$, where \f${\bowtie} \in \{>,\ge,\le,<\}\f$ and \f$c \in \mathbb{N} \f$.

    Let \f$x_0, x_1, \dots x_N\f$ be the variables in the zone. We use \f$x_i\f$ to represent \f$\mathbb{T}_{i,N} = \tau_{i} + \tau_{i+1} \dots \tau_{N}\f$. We note that the first row and column with index 0 of DBM are for the constant 0, and we have to shift the index appropriately.
   */
  class TimedCondition {
    /*!
      @brief A zone to represent the timing constraint
    */
  protected:
    Zone zone;
  public:
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
     * @todo I am not 100% sure about this. Maybe we should add another test case.
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
     * @brief Returns the lower bound of \f$\tau_i + \tau_{i+1} + \dots \tau_{j} \f$.
     */
    Bounds getLowerBound(int i, int j) {
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
    Bounds getUpperBound(int i, int j) {
      assert(0 <= i && i < this->size());
      assert(0 <= j && j < this->size());
      if (j == this->size() - 1) {
        return this->zone.value(i + 1, 0);
      } else {
        return this->zone.value(i + 1, j + 2);

      }
    }

    /*!
      @brief Make a vector of simple timed conditions in this timed condition
     */
    void enumerate(std::vector<TimedCondition>) const;
  };
}

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
    protected Zone zone;

    /*!
      @brief Returns if this timed condition is simple
      
      @pre zone is canonical
    */
    bool isSimple() const {
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
      @brief Make a vector of simple timed conditions in this timed condition
     */
    void enumerate(std::vector<TimedCondition>) const;
  };
}

/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include "zone.hh"

namespace learnta {
  //! @brief A zone constructed by juxtaposing two zones with or without shared variables.
  class JuxtaposedZone : public Zone {
  private:
    std::size_t leftSize;
    std::size_t rightSize;
  public:
    JuxtaposedZone() = default;

    /*!
     * @brief Juxtapose two zones without shared variables
     *
     * Let \f$x_1, x_2, \dots, x_N\f$ be the variables in left and \f$y_1, y_2, \dots, y_M\f$ be the variables in right.
     * The variables \f$z_1, z_2, \dots, z_{N + M}\f$ in the resulting zone is such that
     * - \f$x_i = z_i\f$ if \f$1 \le i \le N\f$ and
     * - \f$y_{i - N} = z_i\f$ if \f$N + 1 \le i \le N + M\f$.
     */
    JuxtaposedZone(const Zone &left, const Zone &right) : leftSize(left.getNumOfVar()), rightSize(right.getNumOfVar()) {
      this->value.resize(leftSize + rightSize + 1, leftSize + rightSize + 1);
      this->value.fill(Bounds(std::numeric_limits<double>::max(), false));
      // Copy the constraints in left
      this->value.block(0, 0, left.value.cols(), left.value.rows()) = left.value;
      // Copy the constraints in right
      this->value.block(left.value.cols(), left.value.rows(), right.value.cols() - 1, right.value.rows() - 1) =
              right.value.block(1, 1, right.value.cols() - 1, right.value.rows() - 1);
      this->value.block(0, left.value.rows(), 1, right.value.rows() - 1) =
              right.value.block(0, 1, 1, right.value.rows() - 1);
      this->value.block(left.value.cols(), 0, right.value.cols() - 1, 1) =
              right.value.block(1, 0, right.value.cols() - 1, 1);

      this->canonize();
    }

    /*!
     * @brief Juxtapose two zones with shared variables
     *
     * Let \f$x_1, x_2, \dots, x_N\f$ be the variables in left and \f$y_1, y_2, \dots, y_M\f$ be the variables in right.
     * Let \f$L\f$ be the size of the variables common in left and right, i.e., \f$x_{N - L + 1} = y_{M - L + 1}, \dots, x_N = y_M\f$.
     * The variables \f$z_1, z_2, \dots, z_{N + M}\f$ in the resulting zone is such that
     * - \f$x_i = z_i\f$ if \f$1 \le i \le N\f$ and
     * - \f$y_{i - M} = z_i\f$ if \f$N + 1 \le i \le N + M - L\f$.
     */
    JuxtaposedZone(const Zone &left, const Zone &right, std::size_t commonVariableSize) :
            leftSize(left.getNumOfVar()), rightSize(right.getNumOfVar()) {
      const auto M = left.getNumOfVar();
      const auto N = right.getNumOfVar();
      const auto L = commonVariableSize;
      const auto resultVariableSize = M + N - L;
      const auto commonBeginIndex = M - L + 1;
      const auto commonBeginInRightIndex = N - L + 1;
      const auto rightBeginIndex = M + 1;
      this->value.resize(resultVariableSize + 1, resultVariableSize + 1);
      this->value.fill(Bounds(std::numeric_limits<double>::max(), false));

      // Copy the constraints in left
      this->value.block(0, 0, left.value.cols(), left.value.rows()) = left.value;

      // Take the conjunction with the constraints in the common part of right
      if (L > 0) {
        this->value.block(commonBeginIndex, commonBeginIndex, L, L) =
                this->value.block(commonBeginIndex, commonBeginIndex, L, L).cwiseMin(
                        right.value.block(commonBeginInRightIndex, commonBeginInRightIndex, L, L));
        this->value.block(0, commonBeginIndex, 1, L) = this->value.block(0, commonBeginIndex, 1, L).cwiseMin(
                right.value.block(0, commonBeginInRightIndex, 1, L));
        this->value.block(commonBeginIndex, 0, L, 1) = this->value.block(commonBeginIndex, 0, L, 1).cwiseMin(
                right.value.block(commonBeginInRightIndex, 0, L, 1));
      }

      // Copy the constraints in the right
      this->value.block(rightBeginIndex, rightBeginIndex, N - L, N - L) = right.value.block(1, 1, N - L, N - L);
      this->value.block(0, rightBeginIndex, 1, N - L) = right.value.block(0, 1, 1, N - L);
      this->value.block(rightBeginIndex, 0, N - L, 1) = right.value.block(1, 0, N - L, 1);

      // Take the conjunction with the constraints between the common and the unique parts of right
      if (L > 0) {
        this->value.block(rightBeginIndex, commonBeginIndex, N - L, L) =
                this->value.block(rightBeginIndex, commonBeginIndex, N - L, L).cwiseMin(
                        right.value.block(1, commonBeginInRightIndex, N - L, L));
        this->value.block(commonBeginIndex, rightBeginIndex, L, N - L) =
                this->value.block(commonBeginIndex, rightBeginIndex, L, N - L).cwiseMin(
                        right.value.block(commonBeginInRightIndex, 1, L, N - L));
      }

      this->canonize();
    }

    //! @brief Add renaming constraints
    void addRenaming(const std::vector<std::pair<std::size_t, std::size_t>> &renaming) {
      for (const auto &pair: renaming) {
        // add T[first][N] == T[second][N]
        Eigen::Index leftIndex = pair.first + 1;
        Eigen::Index rightIndex = pair.second + leftSize + 1;
        this->value(leftIndex, rightIndex) = std::min(this->value(leftIndex, rightIndex), {0, true});
        this->value(rightIndex, leftIndex) = std::min(this->value(rightIndex, leftIndex), {0, true});
      }
      this->canonize();
    }
  };
}
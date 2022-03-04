/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#pragma once

namespace learnta {
  /*!
   * @brief Interface of the system under learning
   */
  class SUL {
  public:
    /*!
     * @brief The function should be called before feeding each timed word
     */
    virtual void pre() = 0;
    /*!
     * @brief The function should be called after feeding each timed word
     */
    virtual void post() = 0;
    /*!
     * @brief Feed a discrete action
     */
    virtual bool step(char action) = 0;
    /*!
     * @brief Feed time elapse
     */
    virtual bool step(double duration) = 0;
  };
}
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
    virtual void pre() = 0;
    virtual void post() = 0;
    virtual bool step(char action) = 0;
    virtual bool step(double duration) = 0;
  };
}
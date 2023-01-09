/**
 * @author Masaki Waga
 * @date 2022/11/29.
 */

#pragma once
#include <memory>
#include <boost/unordered_map.hpp>

#include "sul.hh"
#include "timed_word.hh"

namespace learnta {
  /*!
   * @brief Interface of a membership oracle
   */
  class MembershipOracle {
  public:
    virtual bool answerQuery(const TimedWord &timedWord) = 0;
    [[nodiscard]] virtual std::size_t count() const = 0;
    virtual ~MembershipOracle() = default;

    virtual std::ostream &printStatistics(std::ostream &stream) const {
      stream << "Number of membership queries: " << this->count() << "\n";

      return stream;
    }
  };

  /*!
   * @brief Membership oracle defined by an SUL
   */
  class SULMembershipOracle final : public MembershipOracle {
  private:
    std::unique_ptr<SUL> sul;
  public:
    explicit SULMembershipOracle(std::unique_ptr<SUL> &&sul) : sul(std::move(sul)) {}

    bool answerQuery(const learnta::TimedWord &timedWord) override {
      sul->pre();
      std::string word = timedWord.getWord();
      std::vector<double> duration = timedWord.getDurations();
      bool result = sul->step(duration[0]);
      for (std::size_t i = 0; i < timedWord.wordSize(); i++) {
        sul->step(word[i]);
        result = sul->step(duration[i + 1]);
      }
      sul->post();

      return result;
    }

    [[nodiscard]] size_t count() const override {
      return this->sul->count();
    }
  };

  /*!
   * @brief Wrapper of a membership oracle to cache the result
   */
  class MembershipOracleCache final : public MembershipOracle {
    std::unique_ptr<MembershipOracle> oracle;
    boost::unordered_map<TimedWord, bool> membershipCache;
    std::size_t countNoCache = 0;

  public:
    explicit MembershipOracleCache(std::unique_ptr<MembershipOracle> &&oracle) : oracle(std::move(oracle)) {}

    bool answerQuery(const TimedWord &timedWord) override {
      ++countNoCache;
      auto it = this->membershipCache.find(timedWord);
      if (it != membershipCache.end()) {
        return it->second;
      }
      const auto result = this->oracle->answerQuery(timedWord);
      this->membershipCache[timedWord] = result;

      return result;
    }

    [[nodiscard]] size_t count() const override {
      return this->oracle->count();
    }

    std::ostream &printStatistics(std::ostream &stream) const override {
      stream << "Number of membership queries: " << countNoCache << "\n";
      stream << "Number of membership queries (with cache): " << this->count() << "\n";

      return stream;
    }
  };
}

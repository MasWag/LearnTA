/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <cassert>

#include <boost/functional/hash.hpp>

namespace learnta {
  /*!
   * @brief A timed word
   *
   * We represent the timed word by a sequence of events and the time elapse between each of them (not timestamps), i.e.,
   * \f$\tau_0 a_1 \tau_1 \dots \tau_{n-1} a_{n} \tau_{n} \f$.
   */
  class TimedWord {
  private:
    std::string word;
    std::vector<double> durations;
  public:
    TimedWord() : durations({0}) {}

    TimedWord(std::string word, std::vector<double> durations) : word(std::move(word)),
                                                                 durations(std::move(durations)) {
      assert(this->word.size() + 1 == this->durations.size());
    }

    /*!
     * @brief Return the concatenation of two timed words
     *
     * Let this be \f$\tau_0 a_1 \tau_1 \dots \tau_{n-1} a_{n} \tau_{n} \f$ and another be \f$\tau'_0 a'_1 \tau'_1 \dots \tau'_{m-1} a'_{m} \tau'_{m} \f$. The result is \f$\tau_0 a_1 \tau_1 \dots \tau_{n-1} a_{n} \tau_{n} + \tau'_0 a'_1 \tau'_1 \dots \tau'_{m-1} a'_{m} \tau'_{m}\f$.
     */
    TimedWord operator+(const TimedWord &another) {
      TimedWord result = *this;
      result.word += another.word;
      auto it = another.durations.begin();
      result.durations.back() += *it++;
      result.durations.reserve(this->durations.size() + another.durations.size() - 1);
      for (; it != another.durations.end(); it++) {
        result.durations.push_back(*it);
      }

      return result;
    }

    /*!
     * @brief Return the concatenation of this timed word and an action
     *
     * Let this be \f$\tau_0 a_1 \tau_1 \dots \tau_{n-1} a_{n} \tau_{n} \f$ and action be \f$a\f$. The result is \f$\tau_0 a_1 \tau_1 \dots \tau_{n-1} a_{n} \tau_{n} a 0 \f$.
     */
    TimedWord operator+(const char action) {
      TimedWord result = *this;
      result.word.push_back(action);
      result.durations.push_back(0);

      return result;
    }

    /*!
     * @brief Return the concatenation of this timed word and a time elapse
     *
     * Let this be \f$\tau_0 a_1 \tau_1 \dots \tau_{n-1} a_{n} \tau_{n} \f$ and duration be \f$t\f$. The result is \f$\tau_0 a_1 \tau_1 \dots \tau_{n-1} a_{n} \tau_{n} + t\f$.
     */
    TimedWord operator+(const double duration) {
      TimedWord result = *this;
      result.durations.back() += duration;

      return result;
    }

    /*!
     * @brief Print the timed word to the stream
     * @param stream The stream to write this timed word
     * @return stream
     */
    std::ostream &print(std::ostream &stream) {
      stream << durations[0];
      for (std::size_t i = 0; i < word.size(); i++) {
        stream << " " << word[i] << " " << durations[i + 1];
      }
      return stream;
    }

    [[nodiscard]] const std::string &getWord() const {
      return word;
    }

    [[nodiscard]] const std::vector<double> &getDurations() const {
      return durations;
    }

    /*!
     * @brief Return the number of the actions in this timed word
     */
    [[nodiscard]] std::size_t wordSize() const {
      return this->word.size();
    }

    bool operator==(const TimedWord &another) const {
      return this->word == another.word && this->durations == another.durations;
    }
  };

  static inline std::ostream &print(std::ostream &os, const learnta::TimedWord &word) {
    os << word.getDurations().front();
    for (std::size_t i = 0; i < word.wordSize(); ++i) {
      os << " " << word.getWord().at(i) << " " << word.getDurations().at(i + 1);
    }
    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const learnta::TimedWord &word) {
    return learnta::print(os, word);
  }

  static inline std::size_t hash_value(const TimedWord &word) {
    return boost::hash_value(std::make_tuple(word.getWord(), word.getDurations()));
  }
}
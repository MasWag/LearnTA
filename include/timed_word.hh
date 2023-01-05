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

#include "common_types.hh"

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

    /*!
     * @brief Constructor of timed words
     *
     * @note When the given word contains unobservable actions, we absorb them
     */
    TimedWord(const std::string& word, const std::vector<double>& durations) {
      assert(word.size() + 1 == durations.size());
      this->word.reserve(word.size());
      this->durations = {durations.front()};
      this->durations.reserve(durations.size());
      for (std::size_t i = 0; i < word.size(); ++i) {
        if (word.at(i) != UNOBSERVABLE) {
          this->word.push_back(word.at(i));
          this->durations.push_back(durations.at(i + 1));
        } else {
          this->durations.back() += durations.at(i + 1);
        }
      }

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
    std::ostream &print(std::ostream &stream) const {
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
     * @brief Construct and return the (tail) accumulated duration.
     *
     * The accumulated duration is a vector representing \f$\mathbb{T}_{i,N}\f$, where \f$N\f$ is the length.
     */
    [[nodiscard]] std::vector<double> getAccumulatedDurations() const {
      std::vector<double> accumulatedDurations;
      accumulatedDurations.resize(durations.size());

      accumulatedDurations.back() = durations.back();
      for (int i = durations.size() - 2; i >= 0; --i) {
        accumulatedDurations.at(i) = accumulatedDurations.at(i + 1) +durations.at(i);
      }
      return accumulatedDurations;
    }

    /*!
     * @breif Return the suffix such that the concatenation with prefix is this
     *
     * @pre prefix must be a prefix of this
     */
    [[nodiscard]] TimedWord getSuffix(const TimedWord& prefix) const {
      const auto suffixWord = this->word.substr(prefix.wordSize());
      auto suffixDurations = std::vector<double>(this->durations.begin() + prefix.wordSize(), this->durations.end());
      suffixDurations.front() -= prefix.getDurations().back();

      return TimedWord{suffixWord, suffixDurations};
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
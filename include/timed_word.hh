/**
 * @author Masaki Waga
 * @date 2022/03/04.
 */

#pragma once
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace learnta {
  /*!
   * @brief Timed word represented by a sequence of events and the time elapse between each of them (not timestamps).
   */
  class TimedWord {
  private:
    std::string word;
    std::vector<double> durations;
  public:
    TimedWord() : durations({0}) {}

    TimedWord(std::string word, std::vector<double> durations) : word(std::move(word)), durations(std::move(durations)) {}

    /*!
     * @brief Return the concatenation of two timed words
     */
    TimedWord operator+(const TimedWord& another) {
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
     * @brief Return the concatenation of two timed words
     */
    TimedWord operator+(const char action) {
      TimedWord result = *this;
      result.word.push_back(action);
      result.durations.push_back(0);

      return result;
    }

    /*!
     * @brief Return the concatenation of two timed words
     */
    TimedWord operator+(const double duration) {
      TimedWord result = *this;
      result.durations.back() += duration;

      return result;
    }

    std::ostream& print(std::ostream& stream) {
      stream << durations[0];
      for (int i = 0; i < word.size(); i++) {
        stream << word[i] << durations[i + 1];
      }
      return stream;
    }

    [[nodiscard]] const std::string &getWord() const {
      return word;
    }

    [[nodiscard]] const std::vector<double> &getDurations() const {
      return durations;
    }
  };
}
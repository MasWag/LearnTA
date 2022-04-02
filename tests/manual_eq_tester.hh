/**
 * @author Masaki Waga
 * @date 2022/04/02.
 */

#pragma once

#include "timed_automaton_runner.hh"

namespace learnta {
  struct ManualEqTester {
    TimedAutomatonRunner expected, hypothesis;

    ManualEqTester(TimedAutomatonRunner expected, TimedAutomatonRunner hypothesis) :
            expected(std::move(expected)), hypothesis(std::move(hypothesis)) {
    }

    void run(std::string word, std::vector<double> durations, int wrongLast = 0) {
      hypothesis.pre();
      expected.pre();
      for (int i = 0; i < word.size(); ++i) {
        if (i + wrongLast <= word.size()) {
          BOOST_CHECK_EQUAL(expected.step(durations.at(i)), hypothesis.step(durations.at(i)));
        } else {
          BOOST_CHECK_NE(expected.step(durations.at(i)), hypothesis.step(durations.at(i)));
        }
        if (i + wrongLast < word.size()) {
          BOOST_CHECK_EQUAL(expected.step(word.at(i)), hypothesis.step(word.at(i)));
        } else {
          BOOST_CHECK_NE(expected.step(word.at(i)), hypothesis.step(word.at(i)));
        }
      }
      if (durations.size() > word.size()) {
        if (wrongLast <= 0) {
          BOOST_CHECK_EQUAL(expected.step(durations.back()), hypothesis.step(durations.back()));
        } else {
          BOOST_CHECK_NE(expected.step(durations.back()), hypothesis.step(durations.back()));
        }
      }
      hypothesis.post();
      expected.post();
    }

  };
}
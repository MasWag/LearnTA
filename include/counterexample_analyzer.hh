/**
 * @author Masaki Waga
 * @date 2022/11/30.
 */

#pragma once

#include "recognizable_languages.hh"
#include "membership_oracle.hh"


namespace learnta {
  /*!
   * @brief Rivest-Schapire-style counterexample analysis
   *
   * @param[in] word The analyzed counterexample
   * @param[in] oracle The membership oracle
   * @param[in] hypothesis The hypothesis recognizable language
   *
   * @pre word is a counterexample. Namely, we should have oracle->answerQuery(word) != hypothesis.contains(word)
   */
  static inline TimedWord analyzeCEX(const TimedWord &word,
                                     MembershipOracle &oracle,
                                     const RecognizableLanguage &hypothesis) {
    std::cout << "hypothesis: " << hypothesis << std::endl;
    std::vector<TimedWord> mappedWords = {word};
    std::vector<TimedWord> suffixes = {TimedWord{}};
    while (!hypothesis.inPrefixes(mappedWords.back())) {
      const auto tripleOpt = hypothesis.split(mappedWords.back());
      assert(tripleOpt);
      suffixes.push_back(tripleOpt->suffix);
      mappedWords.push_back(tripleOpt->apply());
    }
    // Conduct binary search
    bool hypothesisResult = hypothesis.contains(mappedWords.back());
    const auto eval = [&] (const TimedWord &w) -> bool {
      return oracle.answerQuery(w) == hypothesisResult;
    };
    assert(eval(mappedWords.back()));
    assert(!eval(mappedWords.front()));
    auto count = mappedWords.size() / 2;
    auto index = count;
    while (eval(mappedWords.at(index)) == eval(mappedWords.at(index - 1))) {
      count = int(ceil(count * 0.5));
      if (eval(mappedWords.at(index))) {
        index -= count;
      } else {
        index += count;
      }
    }
    assert(eval(mappedWords.at(index)) != eval(mappedWords.at(index - 1)));

    return suffixes.at(index);
  }
}
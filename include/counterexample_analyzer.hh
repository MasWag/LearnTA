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
  static inline std::optional<TimedWord> analyzeCEX(const TimedWord &word,
                                                   MembershipOracle &oracle,
                                                   const RecognizableLanguage &hypothesis,
                                                   const std::vector<BackwardRegionalElementaryLanguage> &currentSuffixes = {}) {
    BOOST_LOG_TRIVIAL(debug) << "hypothesis: " << hypothesis;
    std::vector<TimedWord> mappedWords = {word};
    std::vector<TimedWord> suffixes = {TimedWord{}};
    std::vector<SingleMorphism> morphisms;
    while (!hypothesis.inPrefixes(mappedWords.back())) {
      const auto tripleOpt = hypothesis.split(mappedWords.back());
      assert(tripleOpt);
      suffixes.push_back(tripleOpt->suffix);
      morphisms.push_back(tripleOpt->morphism);
      mappedWords.push_back(tripleOpt->apply());
    }
    // Conduct binary search
    bool hypothesisResult = hypothesis.contains(mappedWords.back());
    const auto eval = [&] (const TimedWord &w) -> bool {
      return oracle.answerQuery(w) == hypothesisResult;
    };
    assert(eval(mappedWords.back()));
    assert(!eval(mappedWords.front()));
    if (eval(mappedWords.front())) {
      BOOST_LOG_TRIVIAL(error) << "Something is wrong with the hypothesis: " << hypothesis;
      for (const auto &morphism: morphisms) {
        BOOST_LOG_TRIVIAL(error) << "Morphism: " << morphism;
      }
      for (const auto &mappedWord: mappedWords) {
        BOOST_LOG_TRIVIAL(error) << "mappedWord: " << mappedWord;
      }
      abort();
    }
    for (std::size_t index = 0; index + 1 < mappedWords.size(); ++index) {
      if (eval(mappedWords.at(index)) != eval(mappedWords.at(index + 1))) {
        if (std::all_of(currentSuffixes.begin(), currentSuffixes.end(), [&](const ElementaryLanguage &suffix) {
          return !suffix.contains(suffixes.at(index + 1));
        })) {
          return suffixes.at(index + 1);
        } else {
          BOOST_LOG_TRIVIAL(debug) << suffixes.at(index + 1) << " is a counterexample but not fresh!!";
        }
      }
    }
    return std::nullopt;
  }
}
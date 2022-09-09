/**
 * @author Masaki Waga
 * @date 2022/03/15.
 */

#pragma once

#include "equivalence_oracle.hh"
#include "intersection.hh"
#include "ta2za.hh"
#include "timed_automaton_runner.hh"

#include <utility>

namespace learnta {
  /*!
   * @brief Equivalence oracle with a timed automaton recognizing the complement of the target language
   *
   * @note This is not perfectly reliable because this does not work when the transition of the constructed DTA is not total
   */
  class ComplementTimedAutomataEquivalenceOracle : public EquivalenceOracle {
  private:
    TimedAutomaton target;
    TimedAutomaton complement;
    std::vector<Alphabet> alphabet;

    /*!
     * @brief Check if the language recognized by the target DTA is a subset of that of the hypothesis DTA.
     */
    [[nodiscard]] std::optional<TimedWord> subset(TimedAutomaton hypothesis) const {
      TimedAutomaton intersection;
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      BOOST_LOG_TRIVIAL(debug) << "subset: hypothesis\n" << hypothesis;
      intersectionTA(complement, hypothesis, intersection, toIState);
      ZoneAutomaton zoneAutomaton;
      intersection.simplifyStrong();
      BOOST_LOG_TRIVIAL(debug) << "subset: before ta2za";
      BOOST_LOG_TRIVIAL(debug) << "Number of states: " << intersection.stateSize();
      BOOST_LOG_TRIVIAL(debug) << "Number of clock: " << intersection.clockSize();
      ta2za(intersection, zoneAutomaton);
      BOOST_LOG_TRIVIAL(debug) << "subset: after ta2za";

      return zoneAutomaton.sampleWithMemo();
    }

    /*!
     * @brief Check if the language recognized by the target DTA is a superset of that of the hypothesis DTA.
     */
    [[nodiscard]] std::optional<TimedWord> superset(const TimedAutomaton& hypothesis) const {
      TimedAutomaton intersection;
      boost::unordered_map<std::pair<TAState *, TAState *>, std::shared_ptr<TAState>> toIState;
      const auto complementedHypothesis = hypothesis.complement(this->alphabet);
      BOOST_LOG_TRIVIAL(debug) << "superset: complemented hypothesis\n" << complementedHypothesis;
      intersectionTA(target, complementedHypothesis, intersection, toIState);
      ZoneAutomaton zoneAutomaton;
      intersection.simplifyStrong();
      BOOST_LOG_TRIVIAL(debug) << "superset: before ta2za";
      BOOST_LOG_TRIVIAL(debug) << "Number of states: " << intersection.stateSize();
      BOOST_LOG_TRIVIAL(debug) << "Number of clock: " << intersection.clockSize();
      ta2za(intersection.simplify(), zoneAutomaton);
      BOOST_LOG_TRIVIAL(debug) << "superset: after ta2za";

      return zoneAutomaton.sampleWithMemo();
    }

  public:
    /*!
     * @param[in] complement A timed automaton recognizing the complement of the target language
     */
    ComplementTimedAutomataEquivalenceOracle(TimedAutomaton target, TimedAutomaton complement,
                                             std::vector<Alphabet> alphabet) : target(std::move(target)),
                                                                               complement(std::move(complement)),
                                                                               alphabet(std::move(alphabet)) {
      BOOST_LOG_TRIVIAL(debug) << "Target DTA: \n" << this->target;
      BOOST_LOG_TRIVIAL(debug) << "Complemented target DTA: \n" << this->complement;
    }

    /*!
     * @brief Make an equivalence query
     */
    [[nodiscard]] std::optional<TimedWord> findCounterExample(const TimedAutomaton &hypothesis) const override {
      auto subCounterExample = subset(hypothesis);
      if (subCounterExample) {
        // Confirm that the generated counterexample is really a counterexample
        TimedAutomatonRunner targetRunner{this->target};
        TimedAutomatonRunner hypothesisRunner{hypothesis};
        targetRunner.pre();
        hypothesisRunner.pre();
        for (int i = 0; i < subCounterExample->wordSize(); ++i) {
          targetRunner.step(subCounterExample->getDurations().at(i));
          hypothesisRunner.step(subCounterExample->getDurations().at(i));
          targetRunner.step(subCounterExample->getWord().at(i));
          hypothesisRunner.step(subCounterExample->getWord().at(i));

        }
        assert(targetRunner.step(subCounterExample->getDurations().back()) != hypothesisRunner.step(subCounterExample->getDurations().back()));
        targetRunner.post();
        hypothesisRunner.post();

        return subCounterExample;
      }
      auto supCounterExample = superset(hypothesis);
      if (supCounterExample) {
        // Confirm that the generated counterexample is really a counterexample
        TimedAutomatonRunner targetRunner{this->target};
        TimedAutomatonRunner hypothesisRunner{hypothesis};
        targetRunner.pre();
        hypothesisRunner.pre();
        for (int i = 0; i < supCounterExample->wordSize(); ++i) {
          targetRunner.step(supCounterExample->getDurations().at(i));
          hypothesisRunner.step(supCounterExample->getDurations().at(i));
          targetRunner.step(supCounterExample->getWord().at(i));
          hypothesisRunner.step(supCounterExample->getWord().at(i));
        }
        assert(targetRunner.step(supCounterExample->getDurations().back()) != hypothesisRunner.step(supCounterExample->getDurations().back()));
        targetRunner.post();
        hypothesisRunner.post();
      }

      return supCounterExample;
    }
  };
}
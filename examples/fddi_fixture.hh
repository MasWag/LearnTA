/**
 * @author Masaki Waga
 * @date 2023/01/24.
 */

#pragma once


#pragma once

#include <vector>

#include "common_types.hh"
#include "timed_automaton.hh"

/*!
 * @brief Fixture of the FDDI benchmark, originally from [DOTY'96].
 *
 * We use a variant of the FDDI benchmark in https://github.com/ticktac-project/benchmarks/tree/master/fddi/tchecker.
 * We focus on the FDDI benchmark with two clients.
 *
 * - [DOTY'96]: C. Daws, A. Oliveiro, S. Tripakis, and S. Yovine. The tool Kronos, Hybrid Systems III, 1996.
 */
struct FDDIFixture {
  /*!
   * The mapping of the alphabet from the original benchmark is as follows
   *
   * - P1@TT, R@TT1: a
   * - P1@RT, R@RT1: b
   * - P2@TT, R@TT2: c
   * - P2@RT, R@RT2: d
   * - P1@tau: e
   * - P2@tau: e
   */
  const std::vector<learnta::Alphabet> alphabet = {'a', 'b', 'c', 'd', 'e'};
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  /*!
   * @brief The constructor
   */
  explicit FDDIFixture(int scale = 20) {
    using namespace learnta;
    const std::size_t processSize = 8;
    const std::size_t ringSize = 4;
    // Generate the states
    targetAutomaton.states.reserve(processSize * processSize * ringSize);
    for (std::size_t i = 0; i < processSize * processSize * ringSize; ++i) {
      targetAutomaton.states.push_back(std::make_shared<learnta::TAState>(true));
    }
    const auto toState = [&](const std::size_t p1, const std::size_t p2, const std::size_t ring) {
      assert(p1 < processSize);
      assert(p2 < processSize);
      assert(ring < ringSize);
      return targetAutomaton.states.at(p1 * processSize * ringSize + p2 * ringSize + ring);
    };

    // Define the alphabet
    const learnta::Alphabet P1TT = 'a';
    const learnta::Alphabet P1RT = 'b';
    const learnta::Alphabet P2TT = 'c';
    const learnta::Alphabet P2RT = 'd';
    const learnta::Alphabet P1tau = 'e';
    const learnta::Alphabet P2tau = 'e';

    // Define the clock variables
    const auto trt1 = learnta::ConstraintMaker(0);
    const auto xA1 = learnta::ConstraintMaker(1);
    const auto xB1 = learnta::ConstraintMaker(2);
    const auto trt2 = learnta::ConstraintMaker(3);
    const auto xA2 = learnta::ConstraintMaker(4);
    const auto xB2 = learnta::ConstraintMaker(5);
    const auto t = learnta::ConstraintMaker(6);

    // Construct transitions labelled with P1@TT
    for (const auto p2: {0, 4}) {
      const auto ring = 0, nextRing = 1;
      {
        const auto p1 = 0;
        learnta::TATransition::Resets resets;
        resets.emplace_back(trt1, 0.0);
        resets.emplace_back(xB1, 0.0);
        toState(p1, p2, ring)->next[P1TT].reserve(2);
        toState(p1, p2, ring)->next[P1TT].emplace_back(toState(1, p2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt1 >= scale * 5, t <= 0, t >= 0});
        toState(p1, p2, ring)->next[P1TT].emplace_back(toState(2, p2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt1 < scale * 5, t <= 0, t >= 0});
      }
      {
        const auto p1 = 4;
        learnta::TATransition::Resets resets;
        resets.emplace_back(trt1, 0.0);
        resets.emplace_back(xA1, 0.0);
        toState(p1, p2, ring)->next[P1TT].reserve(2);
        toState(p1, p2, ring)->next[P1TT].emplace_back(toState(5, p2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt1 >= scale * 5, t <= 0, t >= 0});
        toState(p1, p2, ring)->next[P1TT].emplace_back(toState(6, p2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt1 < scale * 5, t <= 0, t >= 0});
      }
    }

    // Construct transitions labelled with P1@RT
    for (const auto p2: {0, 4}) {
      const auto ring = 1, nextRing = 2;
      // Transitions from P1@q1 and P1@q5
      for (const auto p1: {1, 5}) {
        const auto nextP1 = (p1 + 3) % 8;
        learnta::TATransition::Resets resets;
        resets.emplace_back(t, 0.0);
        toState(p1, p2, ring)->next[P1RT].emplace_back(toState(nextP1, p2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt1 >= scale, trt1 <= scale});
      }
      // Transitions from P1@q3 and P1@q7
      for (const auto p1: {3, 7}) {
        const auto nextP1 = (p1 + 1) % 8;
        learnta::TATransition::Resets resets;
        resets.emplace_back(t, 0.0);
        toState(p1, p2, ring)->next[P1RT].emplace_back(toState(nextP1, p2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{(p1 == 3 ? xA1 : xB1) <= scale * 6});
      }
    }

    // Construct transitions labelled with P1@tau
    for (const auto p2: {0, 4}) {
      const auto ring = 1;
      // Transitions from P1@q1 and P1@q5
      for (const auto p1: {2, 6}) {
        const auto nextP1 = p1 + 1;
        learnta::TATransition::Resets resets;
        toState(p1, p2, ring)->next[P1tau].emplace_back(toState(nextP1, p2, ring).get(), resets,
                                                        std::vector<learnta::Constraint>{trt1 >= scale, trt1 <= scale});
      }
    }

    // Construct transitions labelled with P2@TT
    for (const auto p1: {0, 4}) {
      const auto ring = 2, nextRing = 3;
      {
        const auto p2 = 0;
        learnta::TATransition::Resets resets;
        resets.emplace_back(trt2, 0.0);
        resets.emplace_back(xB2, 0.0);
        toState(p1, p2, ring)->next[P2TT].reserve(2);
        toState(p1, p2, ring)->next[P2TT].emplace_back(toState(p1, 1, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt2 >= scale * 5, t <= 0, t >= 0});
        toState(p1, p2, ring)->next[P2TT].emplace_back(toState(p1, 2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt2 < scale * 5, t <= 0, t >= 0});
      }
      {
        const auto p2 = 4;
        learnta::TATransition::Resets resets;
        resets.emplace_back(trt2, 0.0);
        resets.emplace_back(xA2, 0.0);
        toState(p1, p2, ring)->next[P2TT].reserve(2);
        toState(p1, p2, ring)->next[P2TT].emplace_back(toState(p1, 5, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt2 >= scale * 5, t <= 0, t >= 0});
        toState(p1, p2, ring)->next[P2TT].emplace_back(toState(p1, 6, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt2 < scale * 5, t <= 0, t >= 0});
      }
    }

    // Construct transitions labelled with P2@RT
    for (const auto p1: {0, 4}) {
      const auto ring = 3, nextRing = 0;
      // Transitions from P2@q1 and P2@q5
      for (const auto p2: {1, 5}) {
        const auto nextP2 = (p2 + 3) % 8;
        learnta::TATransition::Resets resets;
        resets.emplace_back(t, 0.0);
        toState(p1, p2, ring)->next[P2RT].emplace_back(toState(p1, nextP2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{trt2 >= scale, trt2 <= scale});
      }
      // Transitions from P2@q3 and P2@q7
      for (const auto p2: {3, 7}) {
        const auto nextP2 = (p2 + 1) % 8;
        learnta::TATransition::Resets resets;
        resets.emplace_back(t, 0.0);
        toState(p1, p2, ring)->next[P2RT].emplace_back(toState(p1, nextP2, nextRing).get(), resets,
                                                       std::vector<learnta::Constraint>{(p2 == 3 ? xA2 : xB2) <= scale * 6});
      }
    }

    // Construct transitions labelled with P2@tau
    for (const auto p1: {0, 4}) {
      const auto ring = 3;
      // Transitions from P2@q2 and P2@q6
      for (const auto p2: {2, 6}) {
        const auto nextP2 = p2 + 1;
        learnta::TATransition::Resets resets;
        toState(p1, p2, ring)->next[P2tau].emplace_back(toState(p1, nextP2, ring).get(), resets,
                                                        std::vector<learnta::Constraint>{trt2 >= scale, trt2 <= scale});
      }
    }

    targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
    targetAutomaton.maxConstraints = TimedAutomaton::makeMaxConstants(targetAutomaton.states);

    // simplify the target DTA
    targetAutomaton.simplifyStrong();
    targetAutomaton.simplifyWithZones();

    // Construct the complement DTA
    complementTargetAutomaton = targetAutomaton.complement(this->alphabet);
    complementTargetAutomaton.simplifyStrong();
    complementTargetAutomaton.simplifyWithZones();
  }
};
/**
 * @author Masaki Waga
 * @date 2023/01/25.
 */

#pragma once


#pragma once

#include <vector>

#include "common_types.hh"
#include "timed_automaton.hh"

/*!
 * @brief Fixture of the Fischer benchmark, originally from [AL'94].
 *
 * We use a variant of the Fischer benchmark in https://github.com/ticktac-project/benchmarks/tree/master/fischer/tchecker.
 * We focus on the Fischer protocol with three processes.
 *
 * - [AL'94]: Martin Abadi and Leslie Lamport, An Old-Fashioned Recipe for Real Time, ACM Transactions on Programming Languages and Systems, 16(5), pp. 1543-1571, 1994.
 */
struct FischerFixture {
  /*!
   * The mapping of the alphabet from the original benchmark is as follows
   *
   * - P1@tau: a
   * - P2@tau: b
   * - P3@tau: c
   */
  const std::vector<learnta::Alphabet> alphabet = {'a', 'b', 'c'};
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  /*!
   * @brief The constructor
   */
  explicit FischerFixture(int scale = 10) {
    using namespace learnta;
    const std::size_t size = 4;
    enum State {
      A = 0,
      REQ = 1,
      WAIT = 2,
      CS = 3
    };
    const std::array<State, 4> states = {State::A, State::REQ, State::WAIT, State::CS};
    // Generate the states
    targetAutomaton.states.reserve(size * size * size * 4);
    for (std::size_t i = 0; i < size * size * size * 4; ++i) {
      targetAutomaton.states.push_back(std::make_shared<learnta::TAState>(true));
    }
    const auto toState = [&](const std::size_t p1, const std::size_t p2, const std::size_t p3, const size_t id) {
      assert(p1 < size);
      assert(p2 < size);
      assert(p3 < size);
      assert(id < 4);

      return targetAutomaton.states.at(p1 * size * size * size + p2 * size * size + p3 * size + id);
    };

    // Define the clock variables
    const std::array<learnta::ConstraintMaker, 3> x = {learnta::ConstraintMaker(0),
                                                       learnta::ConstraintMaker(1),
                                                       learnta::ConstraintMaker(2)};

    for (const auto process: {0, 1, 2}) {
      for (const auto pAState: states) {
        for (const auto pBState: states) {
          std::vector<State> source, target;
          source.resize(3);
          target.resize(3);
          source.at((process + 1) % 3) = pAState;
          source.at((process + 2) % 3) = pBState;
          target.at((process + 1) % 3) = pAState;
          target.at((process + 2) % 3) = pBState;

          // Construct transitions from A
          {
            source.at(process) = State::A;
            target.at(process) = State::REQ;
            learnta::TATransition::Resets resets;
            resets.emplace_back(x.at(process), 0.0);
            toState(source.at(0), source.at(1), source.at(2), 0)->next['a' + process].emplace_back(
                    toState(target.at(0), target.at(1), target.at(2), 0).get(),
                    resets, std::vector<learnta::Constraint>{});
          }

          // Construct transitions from req
          {
            source.at(process) = State::REQ;
            target.at(process) = State::WAIT;
            learnta::TATransition::Resets resets;
            resets.emplace_back(x.at(process), 0.0);
            for (const std::size_t sourceId: {0, 1, 2, 3}) {
              toState(source.at(0), source.at(1), source.at(2), sourceId)->next['a' + process].emplace_back(
                      toState(target.at(0), target.at(1), target.at(2), 1).get(),
                      resets, std::vector<learnta::Constraint>{x.at(process) <= scale});
            }
          }

          // Construct transitions from wait to req
          {
            source.at(process) = State::WAIT;
            target.at(process) = State::REQ;
            learnta::TATransition::Resets resets;
            resets.emplace_back(x.at(process), 0.0);
            toState(source.at(0), source.at(1), source.at(2), 0)->next['a' + process].emplace_back(
                    toState(target.at(0), target.at(1), target.at(2), 0).get(),
                    resets, std::vector<learnta::Constraint>{});
          }

          // Construct transitions from wait to cs
          {
            source.at(process) = State::WAIT;
            target.at(process) = State::CS;
            learnta::TATransition::Resets resets;
            toState(source.at(0), source.at(1), source.at(2), 1)->next['a' + process].emplace_back(
                    toState(target.at(0), target.at(1), target.at(2), 1).get(),
                    resets, std::vector<learnta::Constraint>{x.at(process) > scale});
          }

          // Construct transitions from cs to A
          {
            source.at(process) = State::CS;
            target.at(process) = State::A;
            learnta::TATransition::Resets resets;
            toState(source.at(0), source.at(1), source.at(2), 1)->next['a' + process].emplace_back(
                    toState(target.at(0), target.at(1), target.at(2), 0).get(),
                    resets, std::vector<learnta::Constraint>{});
          }
        }
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
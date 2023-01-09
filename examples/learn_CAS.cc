/**
 * @brief An example to learn the car alarm system (CAS) DTA in [APT'20]
 * @author Masaki Waga
 * @date 2022/03/29.
 *
 * - [APT'20]: Aichernig, Bernhard K., Andrea Pferscher, and Martin Tappler. "From passive to active: learning timed automata efficiently." NASA Formal Methods Symposium. Springer, Cham, 2020.
 *
 * Encoding of the events are as follows
 * - l: lock
 * - u: unlock
 * - o: open
 * - c: close
 * - a: armedOn
 * - b: armedOff
 * - f: flashOn
 * - g: flashOff
 * - s: soundOn
 * - t: soundOff
 */

#include <iostream>
#include <memory>
#include <chrono>

#include "timed_automaton.hh"
#include "learner.hh"
#include "timed_automata_equivalence_oracle.hh"
#include "timed_automaton_runner.hh"
#include "equivalance_oracle_chain.hh"
#include "equivalence_oracle_by_test.hh"
#include "experiment_runner.hh"

void run(bool useStaticTests = false) {
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;
  const std::vector<learnta::Alphabet> alphabet = {'l', 'u', 'o', 'c', 'a', 'b', 'f', 'g', 's', 't'};

  // Generate the target DTA
  targetAutomaton.states.resize(15);
  for (int i = 0; i < 14; ++i) {
    targetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(true);
  }
  targetAutomaton.states.at(14) = std::make_shared<learnta::TAState>(false);

  // Transitions
  targetAutomaton.states.at(0)->next['c'].emplace_back();
  targetAutomaton.states.at(0)->next['c'].back().target = targetAutomaton.states.at(2).get();
  targetAutomaton.states.at(0)->next['l'].emplace_back();
  targetAutomaton.states.at(0)->next['l'].back().target = targetAutomaton.states.at(1).get();

  targetAutomaton.states.at(1)->next['u'].emplace_back();
  targetAutomaton.states.at(1)->next['u'].back().target = targetAutomaton.states.at(0).get();
  targetAutomaton.states.at(1)->next['c'].emplace_back();
  targetAutomaton.states.at(1)->next['c'].back().target = targetAutomaton.states.at(3).get();
  targetAutomaton.states.at(1)->next['c'].back().resetVars = {{0, 0.0}};

  targetAutomaton.states.at(2)->next['o'].emplace_back();
  targetAutomaton.states.at(2)->next['o'].back().target = targetAutomaton.states.at(0).get();
  targetAutomaton.states.at(2)->next['l'].emplace_back();
  targetAutomaton.states.at(2)->next['l'].back().target = targetAutomaton.states.at(3).get();
  targetAutomaton.states.at(2)->next['l'].back().resetVars = {{0, 0.0}};

  targetAutomaton.states.at(3)->next['o'].emplace_back();
  targetAutomaton.states.at(3)->next['o'].back().target = targetAutomaton.states.at(1).get();
  targetAutomaton.states.at(3)->next['u'].emplace_back();
  targetAutomaton.states.at(3)->next['u'].back().target = targetAutomaton.states.at(2).get();
  targetAutomaton.states.at(3)->next['a'].emplace_back();
  targetAutomaton.states.at(3)->next['a'].back().target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(3)->next['a'].back().guard = {learnta::ConstraintMaker(0) >= 2};

  targetAutomaton.states.at(4)->next['u'].emplace_back();
  targetAutomaton.states.at(4)->next['u'].back().target = targetAutomaton.states.at(5).get();
  targetAutomaton.states.at(4)->next['o'].emplace_back();
  targetAutomaton.states.at(4)->next['o'].back().target = targetAutomaton.states.at(6).get();

  targetAutomaton.states.at(5)->next['b'].emplace_back();
  targetAutomaton.states.at(5)->next['b'].back().target = targetAutomaton.states.at(2).get();

  targetAutomaton.states.at(6)->next['b'].emplace_back();
  targetAutomaton.states.at(6)->next['b'].back().target = targetAutomaton.states.at(7).get();

  targetAutomaton.states.at(7)->next['f'].emplace_back();
  targetAutomaton.states.at(7)->next['f'].back().target = targetAutomaton.states.at(8).get();

  targetAutomaton.states.at(8)->next['s'].emplace_back();
  targetAutomaton.states.at(8)->next['s'].back().target = targetAutomaton.states.at(9).get();
  targetAutomaton.states.at(8)->next['s'].back().resetVars = {{0, 0.0}};

  targetAutomaton.states.at(9)->next['u'].emplace_back();
  targetAutomaton.states.at(9)->next['u'].back().target = targetAutomaton.states.at(10).get();
  targetAutomaton.states.at(9)->next['u'].back().resetVars = {{0, 0.0}};
  targetAutomaton.states.at(9)->next['t'].emplace_back();
  targetAutomaton.states.at(9)->next['t'].back().target = targetAutomaton.states.at(11).get();
  targetAutomaton.states.at(9)->next['t'].back().guard = {learnta::ConstraintMaker(0) >= 3};
  targetAutomaton.states.at(9)->next['t'].back().resetVars = {{0, 0.0}};

  targetAutomaton.states.at(10)->next['t'].emplace_back();
  targetAutomaton.states.at(10)->next['t'].back().target = targetAutomaton.states.at(12).get();

  targetAutomaton.states.at(11)->next['u'].emplace_back();
  targetAutomaton.states.at(11)->next['u'].back().target = targetAutomaton.states.at(12).get();
  targetAutomaton.states.at(11)->next['g'].emplace_back();
  targetAutomaton.states.at(11)->next['g'].back().target = targetAutomaton.states.at(13).get();
  targetAutomaton.states.at(11)->next['g'].back().guard = {learnta::ConstraintMaker(0) >= 27};

  targetAutomaton.states.at(12)->next['g'].emplace_back();
  targetAutomaton.states.at(12)->next['g'].back().target = targetAutomaton.states.at(0).get();

  targetAutomaton.states.at(13)->next['u'].emplace_back();
  targetAutomaton.states.at(13)->next['u'].back().target = targetAutomaton.states.at(0).get();

  targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
  targetAutomaton.maxConstraints.resize(1);
  targetAutomaton.maxConstraints[0] = 27;

  // Execute the learning
  learnta::ExperimentRunner runner {alphabet, targetAutomaton};
  runner.run();
}

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif
  run(argc > 1);

  return 0;
}

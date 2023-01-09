/**
 * @brief An example to learn the particle controller (PC) DTA in [APT'20]
 * @author Masaki Waga
 * @date 2022/04/04.
 *
 * - [APT'20]: Aichernig, Bernhard K., Andrea Pferscher, and Martin Tappler. "From passive to active: learning timed automata efficiently." NASA Formal Methods Symposium. Springer, Cham, 2020.
 *
 * The figure of the PC DTA can be found in page 57 of https://diglib.tugraz.at/download.php?id=5df79492e0e20&location=browse .
 *
 * Encoding of the events are as follows
 * - s: SpauState!
 * - u: SetPurge?
 * - b: StatusBusy!
 * - a: SetPause?
 * - l: SpulState!
 * - t: SetStandby?
 * - y: StbyState!
 * - e: LeakageTest
 * - c: SlecState
 * - r: ResponseCheck
 * - g: SegaState
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
#include "equivalence_oracle_by_random_test.hh"
#include "experiment_runner.hh"
#include "equivalence_oracle_memo.hh"

void run() {
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;
  const std::vector<learnta::Alphabet> alphabet = {'s', 'u', 'b', 'a', 'l', 't', 'y', 'e', 'c', 'r', 'g'};

  // Generate the target DTA
  targetAutomaton.states.resize(17);
  for (int i = 0; i < 16; ++i) {
    targetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(true);
  }
  targetAutomaton.states.at(16) = std::make_shared<learnta::TAState>(false);

  // Transitions
  targetAutomaton.states.at(0)->next['s'].emplace_back();
  targetAutomaton.states.at(0)->next['s'].back().target = targetAutomaton.states.at(1).get();

  targetAutomaton.states.at(1)->next['u'].emplace_back();
  targetAutomaton.states.at(1)->next['u'].back().target = targetAutomaton.states.at(2).get();
  targetAutomaton.states.at(1)->next['u'].back().resetVars = {{0, 0.0}};
  targetAutomaton.states.at(1)->next['t'].emplace_back();
  targetAutomaton.states.at(1)->next['t'].back().target = targetAutomaton.states.at(3).get();

  targetAutomaton.states.at(2)->next['b'].emplace_back();
  targetAutomaton.states.at(2)->next['b'].back().target = targetAutomaton.states.at(4).get();

  targetAutomaton.states.at(3)->next['b'].emplace_back();
  targetAutomaton.states.at(3)->next['b'].back().target = targetAutomaton.states.at(6).get();

  targetAutomaton.states.at(4)->next['l'].emplace_back();
  targetAutomaton.states.at(4)->next['l'].back().target = targetAutomaton.states.at(5).get();

  targetAutomaton.states.at(5)->next['a'].emplace_back();
  targetAutomaton.states.at(5)->next['a'].back().target = targetAutomaton.states.at(1).get();
  targetAutomaton.states.at(5)->next['s'].emplace_back();
  targetAutomaton.states.at(5)->next['s'].back().target = targetAutomaton.states.at(2).get();
  targetAutomaton.states.at(5)->next['s'].back().guard = {learnta::ConstraintMaker(0) >= 10};
  targetAutomaton.states.at(5)->next['t'].emplace_back();
  targetAutomaton.states.at(5)->next['t'].back().target = targetAutomaton.states.at(6).get();

  targetAutomaton.states.at(6)->next['y'].emplace_back();
  targetAutomaton.states.at(6)->next['y'].back().target = targetAutomaton.states.at(7).get();

  targetAutomaton.states.at(7)->next['u'].emplace_back();
  targetAutomaton.states.at(7)->next['u'].back().target = targetAutomaton.states.at(8).get();
  targetAutomaton.states.at(7)->next['u'].back().resetVars = {{0, 0.0}};
  targetAutomaton.states.at(7)->next['e'].emplace_back();
  targetAutomaton.states.at(7)->next['e'].back().target = targetAutomaton.states.at(9).get();
  targetAutomaton.states.at(7)->next['e'].back().resetVars = {{0, 0.0}};
  targetAutomaton.states.at(7)->next['r'].emplace_back();
  targetAutomaton.states.at(7)->next['r'].back().target = targetAutomaton.states.at(10).get();
  targetAutomaton.states.at(7)->next['r'].back().resetVars = {{0, 0.0}};
  targetAutomaton.states.at(7)->next['a'].emplace_back();
  targetAutomaton.states.at(7)->next['a'].back().target = targetAutomaton.states.at(11).get();

  targetAutomaton.states.at(8)->next['b'].emplace_back();
  targetAutomaton.states.at(8)->next['b'].back().target = targetAutomaton.states.at(12).get();

  targetAutomaton.states.at(9)->next['b'].emplace_back();
  targetAutomaton.states.at(9)->next['b'].back().target = targetAutomaton.states.at(13).get();

  targetAutomaton.states.at(10)->next['b'].emplace_back();
  targetAutomaton.states.at(10)->next['b'].back().target = targetAutomaton.states.at(14).get();

  targetAutomaton.states.at(11)->next['b'].emplace_back();
  targetAutomaton.states.at(11)->next['b'].back().target = targetAutomaton.states.at(0).get();

  targetAutomaton.states.at(12)->next['l'].emplace_back();
  targetAutomaton.states.at(12)->next['l'].back().target = targetAutomaton.states.at(15).get();

  targetAutomaton.states.at(13)->next['c'].emplace_back();
  targetAutomaton.states.at(13)->next['c'].back().target = targetAutomaton.states.at(15).get();

  targetAutomaton.states.at(14)->next['g'].emplace_back();
  targetAutomaton.states.at(14)->next['g'].back().target = targetAutomaton.states.at(15).get();

  targetAutomaton.states.at(15)->next['a'].emplace_back();
  targetAutomaton.states.at(15)->next['a'].back().target = targetAutomaton.states.at(0).get();
  targetAutomaton.states.at(15)->next['t'].emplace_back();
  targetAutomaton.states.at(15)->next['t'].back().target = targetAutomaton.states.at(6).get();
  targetAutomaton.states.at(15)->next['y'].emplace_back();
  targetAutomaton.states.at(15)->next['y'].back().target = targetAutomaton.states.at(7).get();
  targetAutomaton.states.at(15)->next['y'].back().guard = {learnta::ConstraintMaker(0) >= 10};

  targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
  targetAutomaton.maxConstraints.resize(1);
  targetAutomaton.maxConstraints[0] = 10;

  // Execute the learning
  learnta::ExperimentRunner runner {alphabet, targetAutomaton};
  runner.run();
}

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  run();

  return 0;
}

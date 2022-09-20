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
  const std::vector<Alphabet> alphabet = {'s', 'u', 'b', 'a', 'l', 't', 'y', 'e', 'c', 'r', 'g'};

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

  // Generate the complement of the target DTA
  complementTargetAutomaton = targetAutomaton.complement(alphabet);

  // Construct the learner
  auto sul = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner(targetAutomaton));
  auto memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(sul));
  auto eqOracle = std::make_unique<learnta::EquivalenceOracleChain>();
  auto eqOracleByTest = std::make_unique<learnta::EquivalenceOracleByTest>(targetAutomaton);
  // Equivalence query by static string to make the evaluation stable
  // These strings are generated in macOS but the equivalence query returns a different set of counter examples.
  // This deviation is probably because of the difference in the address handling, which is used in sorting.
  // eqOracleByTest->push_back(learnta::TimedWord{"claubcl", {0, 0, 2, 0, 0, 0, 0, 0}});
  // eqOracleByTest->push_back(learnta::TimedWord{"claubcf", {0, 0, 2, 0, 0, 0, 0, 0}});

  eqOracle->push_back(std::move(eqOracleByTest));
  eqOracle->push_back(
          std::make_unique<learnta::ComplementTimedAutomataEquivalenceOracle>(
                  targetAutomaton, complementTargetAutomaton, alphabet));
  learnta::Learner learner{alphabet, std::move(memOracle),
                           std::make_unique<learnta::EquivalenceOracleMemo>(std::move(eqOracle), targetAutomaton)};

  // Run the learning
  std::cout << "Start Learning!!" << std::endl;
  const auto startTime = std::chrono::system_clock::now(); // Current time
  const auto hypothesis = learner.run();
  const auto endTime = std::chrono::system_clock::now(); // End time

  std::cout << "Learning Finished!!" << std::endl;
  std::cout << "The learned DTA is as follows\n" << hypothesis << std::endl;
  learner.printStatistics(std::cout);
  std::cout << "Execution Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()
            << " [ms]" << std::endl;
}

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  run();

  return 0;
}

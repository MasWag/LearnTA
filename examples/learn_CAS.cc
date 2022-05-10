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

void run(bool useStaticTests = false) {
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;
  const std::vector<Alphabet> alphabet = {'l', 'u', 'o', 'c', 'a', 'b', 'f', 'g', 's', 't'};

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
  targetAutomaton.states.at(1)->next['c'].back().resetVars = {{0, std::nullopt}};

  targetAutomaton.states.at(2)->next['o'].emplace_back();
  targetAutomaton.states.at(2)->next['o'].back().target = targetAutomaton.states.at(0).get();
  targetAutomaton.states.at(2)->next['l'].emplace_back();
  targetAutomaton.states.at(2)->next['l'].back().target = targetAutomaton.states.at(3).get();
  targetAutomaton.states.at(2)->next['l'].back().resetVars = {{0, std::nullopt}};

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
  targetAutomaton.states.at(8)->next['s'].back().resetVars = {{0, std::nullopt}};

  targetAutomaton.states.at(9)->next['u'].emplace_back();
  targetAutomaton.states.at(9)->next['u'].back().target = targetAutomaton.states.at(10).get();
  targetAutomaton.states.at(9)->next['u'].back().resetVars = {{0, std::nullopt}};
  targetAutomaton.states.at(9)->next['t'].emplace_back();
  targetAutomaton.states.at(9)->next['t'].back().target = targetAutomaton.states.at(11).get();
  targetAutomaton.states.at(9)->next['t'].back().guard = {learnta::ConstraintMaker(0) >= 3};
  targetAutomaton.states.at(9)->next['t'].back().resetVars = {{0, std::nullopt}};

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

  // Generate the complement of the target DTA
  complementTargetAutomaton.states.resize(15);
  for (int i = 0; i < 14; ++i) {
    complementTargetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(false);
  }
  complementTargetAutomaton.states.at(14) = std::make_shared<learnta::TAState>(true);

  // Transitions
  complementTargetAutomaton.states.at(0)->next['l'].emplace_back();
  complementTargetAutomaton.states.at(0)->next['l'].back().target = complementTargetAutomaton.states.at(1).get();
  complementTargetAutomaton.states.at(0)->next['c'].emplace_back();
  complementTargetAutomaton.states.at(0)->next['c'].back().target = complementTargetAutomaton.states.at(2).get();

  complementTargetAutomaton.states.at(1)->next['u'].emplace_back();
  complementTargetAutomaton.states.at(1)->next['u'].back().target = complementTargetAutomaton.states.at(0).get();
  complementTargetAutomaton.states.at(1)->next['c'].emplace_back();
  complementTargetAutomaton.states.at(1)->next['c'].back().target = complementTargetAutomaton.states.at(3).get();
  complementTargetAutomaton.states.at(1)->next['c'].back().resetVars = {{0, std::nullopt}};

  complementTargetAutomaton.states.at(2)->next['o'].emplace_back();
  complementTargetAutomaton.states.at(2)->next['o'].back().target = complementTargetAutomaton.states.at(0).get();
  complementTargetAutomaton.states.at(2)->next['l'].emplace_back();
  complementTargetAutomaton.states.at(2)->next['l'].back().target = complementTargetAutomaton.states.at(3).get();
  complementTargetAutomaton.states.at(2)->next['l'].back().resetVars = {{0, std::nullopt}};

  complementTargetAutomaton.states.at(3)->next['o'].emplace_back();
  complementTargetAutomaton.states.at(3)->next['o'].back().target = complementTargetAutomaton.states.at(1).get();
  complementTargetAutomaton.states.at(3)->next['u'].emplace_back();
  complementTargetAutomaton.states.at(3)->next['u'].back().target = complementTargetAutomaton.states.at(2).get();
  complementTargetAutomaton.states.at(3)->next['a'].emplace_back();
  complementTargetAutomaton.states.at(3)->next['a'].back().target = complementTargetAutomaton.states.at(4).get();
  complementTargetAutomaton.states.at(3)->next['a'].back().guard = {learnta::ConstraintMaker(0) >= 2};
  complementTargetAutomaton.states.at(3)->next['a'].emplace_back();
  complementTargetAutomaton.states.at(3)->next['a'].back().target = complementTargetAutomaton.states.at(14).get();
  complementTargetAutomaton.states.at(3)->next['a'].back().guard = {learnta::ConstraintMaker(0) < 2};

  complementTargetAutomaton.states.at(4)->next['u'].emplace_back();
  complementTargetAutomaton.states.at(4)->next['u'].back().target = complementTargetAutomaton.states.at(5).get();
  complementTargetAutomaton.states.at(4)->next['o'].emplace_back();
  complementTargetAutomaton.states.at(4)->next['o'].back().target = complementTargetAutomaton.states.at(6).get();

  complementTargetAutomaton.states.at(5)->next['b'].emplace_back();
  complementTargetAutomaton.states.at(5)->next['b'].back().target = complementTargetAutomaton.states.at(2).get();

  complementTargetAutomaton.states.at(6)->next['b'].emplace_back();
  complementTargetAutomaton.states.at(6)->next['b'].back().target = complementTargetAutomaton.states.at(7).get();

  complementTargetAutomaton.states.at(7)->next['f'].emplace_back();
  complementTargetAutomaton.states.at(7)->next['f'].back().target = complementTargetAutomaton.states.at(8).get();

  complementTargetAutomaton.states.at(8)->next['s'].emplace_back();
  complementTargetAutomaton.states.at(8)->next['s'].back().target = complementTargetAutomaton.states.at(9).get();
  complementTargetAutomaton.states.at(8)->next['s'].back().resetVars = {{0, std::nullopt}};

  complementTargetAutomaton.states.at(9)->next['u'].emplace_back();
  complementTargetAutomaton.states.at(9)->next['u'].back().target = complementTargetAutomaton.states.at(10).get();
  complementTargetAutomaton.states.at(9)->next['u'].back().resetVars = {{0, std::nullopt}};
  complementTargetAutomaton.states.at(9)->next['t'].emplace_back();
  complementTargetAutomaton.states.at(9)->next['t'].back().target = complementTargetAutomaton.states.at(11).get();
  complementTargetAutomaton.states.at(9)->next['t'].back().guard = {learnta::ConstraintMaker(0) >= 3};
  complementTargetAutomaton.states.at(9)->next['t'].back().resetVars = {{0, std::nullopt}};
  complementTargetAutomaton.states.at(9)->next['t'].emplace_back();
  complementTargetAutomaton.states.at(9)->next['t'].back().target = complementTargetAutomaton.states.at(14).get();
  complementTargetAutomaton.states.at(9)->next['t'].back().guard = {learnta::ConstraintMaker(0) < 3};

  complementTargetAutomaton.states.at(10)->next['t'].emplace_back();
  complementTargetAutomaton.states.at(10)->next['t'].back().target = complementTargetAutomaton.states.at(12).get();

  complementTargetAutomaton.states.at(11)->next['u'].emplace_back();
  complementTargetAutomaton.states.at(11)->next['u'].back().target = complementTargetAutomaton.states.at(12).get();
  complementTargetAutomaton.states.at(11)->next['g'].emplace_back();
  complementTargetAutomaton.states.at(11)->next['g'].back().target = complementTargetAutomaton.states.at(13).get();
  complementTargetAutomaton.states.at(11)->next['g'].back().guard = {learnta::ConstraintMaker(0) >= 27};
  complementTargetAutomaton.states.at(11)->next['g'].emplace_back();
  complementTargetAutomaton.states.at(11)->next['g'].back().target = complementTargetAutomaton.states.at(14).get();
  complementTargetAutomaton.states.at(11)->next['g'].back().guard = {learnta::ConstraintMaker(0) < 27};

  complementTargetAutomaton.states.at(12)->next['g'].emplace_back();
  complementTargetAutomaton.states.at(12)->next['g'].back().target = complementTargetAutomaton.states.at(0).get();

  complementTargetAutomaton.states.at(13)->next['u'].emplace_back();
  complementTargetAutomaton.states.at(13)->next['u'].back().target = complementTargetAutomaton.states.at(0).get();

  // If the transition is empty, we make a transition to the sink state
  for (auto &state: complementTargetAutomaton.states) {
    for (const auto &action: alphabet) {
      if (state->next.find(action) == state->next.end()) {
        state->next[action].emplace_back();
        state->next.at(action).back().target = complementTargetAutomaton.states.at(14).get();
      }
    }
  }

  complementTargetAutomaton.initialStates.push_back(complementTargetAutomaton.states.at(0));
  complementTargetAutomaton.maxConstraints.resize(1);
  complementTargetAutomaton.maxConstraints[0] = 27;

  // Construct the learner
  auto sul = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner(targetAutomaton));
  auto memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(sul));
  auto eqOracle = std::make_unique<learnta::EquivalenceOracleChain>();
  auto eqOracleByTest = std::make_unique<learnta::EquivalenceOracleByTest>(targetAutomaton);
  // Equivalence query by static string to make the evaluation stable
  // These strings are generated in macOS but the equivalence query returns a different set of counter examples.
  // This deviation is probably because of the difference in the address handling, which is used in sorting.
  eqOracleByTest->push_back(learnta::TimedWord{"cc", {0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"ll", {0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"cla", {0, 0, 2, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"lca", {2, 0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"lca", {0, 2, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"clauu", {0, 0, 2, 0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"claobl", {0, 0, 2, 0, 0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"claobff", {0, 0, 2, 0, 0, 0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"claobfsc", {0, 0, 2, 0, 0, 0, 0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"claobfsus", {0, 0, 2, 0, 0, 0, 0, 0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"claobfsutt", {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"claobfst", {0, 0, 2, 0, 0, 0, 0, 3, 0}});
  eqOracleByTest->push_back(learnta::TimedWord{"claobfstg", {0, 0, 2, 0, 0, 0, 0, 3, 27, 0}});
  // eqOracleByTest->push_back(learnta::TimedWord{"claubcl", {0, 0, 2, 0, 0, 0, 0, 0}});
  // eqOracleByTest->push_back(learnta::TimedWord{"claubcf", {0, 0, 2, 0, 0, 0, 0, 0}});

  if (useStaticTests) {
    std::cout << "Use static test set in the equivalence query for stability" << std::endl;
    eqOracle->push_back(std::move(eqOracleByTest));
  }
  eqOracle->push_back(
          std::make_unique<learnta::ComplementTimedAutomataEquivalenceOracle>(
                  targetAutomaton, complementTargetAutomaton, alphabet));
  learnta::Learner learner{alphabet, std::move(memOracle), std::move(eqOracle)};

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
  run(argc > 1);

  return 0;
}

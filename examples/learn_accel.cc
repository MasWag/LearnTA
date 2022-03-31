/**
 * @brief An example to learn a variant of the Accel DTA in [WHS'17]
 * @author Masaki Waga
 * @date 2022/03/28.
 *
 * - [WHS'17]: Waga, Masaki, Ichiro Hasuo, and Kohei Suenaga. "Efficient online timed pattern matching by automata-based skipping." International Conference on Formal Modeling and Analysis of Timed Systems. Springer, Cham, 2017.
 */

#include <iostream>

#include "timed_automaton.hh"
#include "learner.hh"
#include "timed_automata_equivalence_oracle.hh"
#include "timed_automaton_runner.hh"

void run(int scale) {
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  // Generate the target DTA
  targetAutomaton.states.resize(10);
  for (int i = 0; i < 9; ++i) {
    targetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(false);
  }
  targetAutomaton.states.at(9) = std::make_shared<learnta::TAState>(true);
  // Transitions
  targetAutomaton.states.at(0)->next['1'].emplace_back();
  targetAutomaton.states.at(0)->next['1'].back().target = targetAutomaton.states.at(1).get();
  targetAutomaton.states.at(0)->next['h'].emplace_back();
  targetAutomaton.states.at(0)->next['h'].back().target = targetAutomaton.states.at(5).get();

  targetAutomaton.states.at(1)->next['2'].emplace_back();
  targetAutomaton.states.at(1)->next['2'].back().target = targetAutomaton.states.at(2).get();
  targetAutomaton.states.at(1)->next['h'].emplace_back();
  targetAutomaton.states.at(1)->next['h'].back().target = targetAutomaton.states.at(6).get();

  targetAutomaton.states.at(2)->next['3'].emplace_back();
  targetAutomaton.states.at(2)->next['3'].back().target = targetAutomaton.states.at(3).get();
  targetAutomaton.states.at(2)->next['h'].emplace_back();
  targetAutomaton.states.at(2)->next['h'].back().target = targetAutomaton.states.at(7).get();

  targetAutomaton.states.at(3)->next['4'].emplace_back();
  targetAutomaton.states.at(3)->next['4'].back().target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(3)->next['4'].back().guard = {learnta::ConstraintMaker(0) <= scale};
  targetAutomaton.states.at(3)->next['4'].back().resetVars.emplace_back(0, std::nullopt);
  targetAutomaton.states.at(3)->next['h'].emplace_back();
  targetAutomaton.states.at(3)->next['h'].back().target = targetAutomaton.states.at(8).get();

  targetAutomaton.states.at(4)->next['h'].emplace_back();
  targetAutomaton.states.at(4)->next['h'].back().target = targetAutomaton.states.at(9).get();

  targetAutomaton.states.at(5)->next['1'].emplace_back();
  targetAutomaton.states.at(5)->next['1'].back().target = targetAutomaton.states.at(6).get();

  targetAutomaton.states.at(6)->next['2'].emplace_back();
  targetAutomaton.states.at(6)->next['2'].back().target = targetAutomaton.states.at(7).get();

  targetAutomaton.states.at(7)->next['3'].emplace_back();
  targetAutomaton.states.at(7)->next['3'].back().target = targetAutomaton.states.at(8).get();

  targetAutomaton.states.at(8)->next['4'].emplace_back();
  targetAutomaton.states.at(8)->next['4'].back().target = targetAutomaton.states.at(9).get();
  targetAutomaton.states.at(8)->next['4'].back().guard = {learnta::ConstraintMaker(0) <= scale};
  targetAutomaton.states.at(8)->next['4'].back().resetVars.emplace_back(0, std::nullopt);

  targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
  targetAutomaton.maxConstraints.resize(1);
  targetAutomaton.maxConstraints[0] = scale;

  // Generate the complement of the target DTA
  complementTargetAutomaton.states.resize(11);
  for (int i = 0; i < 9; ++i) {
    complementTargetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(true);
  }
  complementTargetAutomaton.states.at(9) = std::make_shared<learnta::TAState>(false);
  // The sink state
  complementTargetAutomaton.states.at(10) = std::make_shared<learnta::TAState>(true);

  complementTargetAutomaton.states.at(0)->next['1'].emplace_back();
  complementTargetAutomaton.states.at(0)->next['1'].back().target = complementTargetAutomaton.states.at(1).get();
  complementTargetAutomaton.states.at(0)->next['h'].emplace_back();
  complementTargetAutomaton.states.at(0)->next['h'].back().target = complementTargetAutomaton.states.at(5).get();

  complementTargetAutomaton.states.at(1)->next['2'].emplace_back();
  complementTargetAutomaton.states.at(1)->next['2'].back().target = complementTargetAutomaton.states.at(2).get();
  complementTargetAutomaton.states.at(1)->next['h'].emplace_back();
  complementTargetAutomaton.states.at(1)->next['h'].back().target = complementTargetAutomaton.states.at(6).get();

  complementTargetAutomaton.states.at(2)->next['3'].emplace_back();
  complementTargetAutomaton.states.at(2)->next['3'].back().target = complementTargetAutomaton.states.at(3).get();
  complementTargetAutomaton.states.at(2)->next['h'].emplace_back();
  complementTargetAutomaton.states.at(2)->next['h'].back().target = complementTargetAutomaton.states.at(7).get();

  complementTargetAutomaton.states.at(3)->next['4'].emplace_back();
  complementTargetAutomaton.states.at(3)->next['4'].back().target = complementTargetAutomaton.states.at(4).get();
  complementTargetAutomaton.states.at(3)->next['4'].back().guard = {learnta::ConstraintMaker(0) <= scale};
  complementTargetAutomaton.states.at(3)->next['4'].back().resetVars.emplace_back(0, std::nullopt);
  complementTargetAutomaton.states.at(3)->next['4'].emplace_back();
  complementTargetAutomaton.states.at(3)->next['4'].back().target = complementTargetAutomaton.states.at(10).get();
  complementTargetAutomaton.states.at(3)->next['4'].back().guard = {learnta::ConstraintMaker(0) > scale};
  complementTargetAutomaton.states.at(3)->next['h'].emplace_back();
  complementTargetAutomaton.states.at(3)->next['h'].back().target = complementTargetAutomaton.states.at(8).get();

  complementTargetAutomaton.states.at(4)->next['h'].emplace_back();
  complementTargetAutomaton.states.at(4)->next['h'].back().target = complementTargetAutomaton.states.at(9).get();

  complementTargetAutomaton.states.at(5)->next['1'].emplace_back();
  complementTargetAutomaton.states.at(5)->next['1'].back().target = complementTargetAutomaton.states.at(6).get();

  complementTargetAutomaton.states.at(6)->next['2'].emplace_back();
  complementTargetAutomaton.states.at(6)->next['2'].back().target = complementTargetAutomaton.states.at(7).get();

  complementTargetAutomaton.states.at(7)->next['3'].emplace_back();
  complementTargetAutomaton.states.at(7)->next['3'].back().target = complementTargetAutomaton.states.at(8).get();

  complementTargetAutomaton.states.at(8)->next['4'].emplace_back();
  complementTargetAutomaton.states.at(8)->next['4'].back().target = complementTargetAutomaton.states.at(9).get();
  complementTargetAutomaton.states.at(8)->next['4'].back().guard = {learnta::ConstraintMaker(0) <= scale};
  complementTargetAutomaton.states.at(8)->next['4'].back().resetVars.emplace_back(0, std::nullopt);
  complementTargetAutomaton.states.at(8)->next['4'].back().target = complementTargetAutomaton.states.at(10).get();
  complementTargetAutomaton.states.at(8)->next['4'].back().guard = {learnta::ConstraintMaker(0) > scale};

  const std::vector<Alphabet> alphabet = {'1', '2', '3', '4', 'h'};

  // If the transition is empty, we make a transition to the sink state
  for (auto& state: complementTargetAutomaton.states) {
    for (const auto& action: alphabet) {
      if (state->next.find(action) == state->next.end()) {
        state->next[action].emplace_back();
        state->next.at(action).back().target = complementTargetAutomaton.states.at(10).get();
      }
    }
  }

  complementTargetAutomaton.initialStates.push_back(complementTargetAutomaton.states.at(0));
  complementTargetAutomaton.maxConstraints.resize(1);
  complementTargetAutomaton.maxConstraints[0] = scale;

  // Construct the learner
  auto sul = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner(targetAutomaton));
  auto memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(sul));
  auto eqOracle = std::unique_ptr<learnta::EquivalenceOracle>(
          new learnta::ComplementTimedAutomataEquivalenceOracle(targetAutomaton, complementTargetAutomaton, alphabet));
  learnta::Learner learner{alphabet, std::move(memOracle), std::move(eqOracle)};

  // Run the learning
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

  std::cout << "Usage: " << argv[0] << " [scales]" << std::endl;
  if (argc == 1) {
    std::cout << "Use the default scale" << std::endl;
    run(10);
  } else {
    for (int i = 1; i < argc; ++i) {
      std::cout << "Use scale = " << argv[i] << std::endl;
      run(atoi(argv[i]));
    }
  }

  return 0;
}

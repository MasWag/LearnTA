/**
 * @author Masaki Waga
 * @date 2022/06/26.
 * @brief Implements the "Unbalanced" benchmark, which is inspired by the unbalanced TRE in [ACM'02]
 */

#include <iostream>

#include "timed_automaton.hh"
#include "learner.hh"
#include "timed_automata_equivalence_oracle.hh"
#include "timed_automaton_runner.hh"
#include "equivalance_oracle_chain.hh"
#include "equivalence_oracle_by_test.hh"

void run(int scale) {
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  // Generate the target DTA
  targetAutomaton.states.resize(5);
  // The initial state
  targetAutomaton.states.at(0) = std::make_shared<learnta::TAState>(false);
  targetAutomaton.states.at(0)->next['a'].resize(1);
  targetAutomaton.states.at(0)->next['b'].resize(1);
  targetAutomaton.states.at(0)->next['c'].resize(1);
  // The state waiting for b
  targetAutomaton.states.at(1) = std::make_shared<learnta::TAState>(false);
  targetAutomaton.states.at(1)->next['a'].resize(1);
  targetAutomaton.states.at(1)->next['b'].resize(3);
  targetAutomaton.states.at(1)->next['c'].resize(1);
  // The state waiting for c
  targetAutomaton.states.at(2) = std::make_shared<learnta::TAState>(false);
  targetAutomaton.states.at(2)->next['a'].resize(1);
  targetAutomaton.states.at(2)->next['b'].resize(1);
  targetAutomaton.states.at(2)->next['c'].resize(3);
  // The accepting state
  targetAutomaton.states.at(3) = std::make_shared<learnta::TAState>(true);
  targetAutomaton.states.at(3)->next['a'].resize(1);
  targetAutomaton.states.at(3)->next['b'].resize(1);
  targetAutomaton.states.at(3)->next['c'].resize(1);
  // The sink state
  targetAutomaton.states.at(4) = std::make_shared<learnta::TAState>(false);
  targetAutomaton.states.at(4)->next['a'].resize(1);
  targetAutomaton.states.at(4)->next['b'].resize(1);
  targetAutomaton.states.at(4)->next['c'].resize(1);
  // Transitions from loc0
  targetAutomaton.states.at(0)->next['a'].at(0).target = targetAutomaton.states.at(1).get();
  targetAutomaton.states.at(0)->next['a'].at(0).resetVars = {{1, 0.0}};
  targetAutomaton.states.at(0)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(0)->next['c'].at(0).target = targetAutomaton.states.at(4).get();
  // Transitions from loc1
  targetAutomaton.states.at(1)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(1)->next['b'].at(0).target = targetAutomaton.states.at(2).get();
  targetAutomaton.states.at(1)->next['b'].at(0).guard = {learnta::ConstraintMaker(0) >= scale,
                                                         learnta::ConstraintMaker(0) <= scale};
  targetAutomaton.states.at(1)->next['b'].at(1).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(1)->next['b'].at(1).guard = {learnta::ConstraintMaker(0) > scale};
  targetAutomaton.states.at(1)->next['b'].at(2).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(1)->next['b'].at(2).guard = {learnta::ConstraintMaker(0) < scale};
  targetAutomaton.states.at(1)->next['c'].at(0).target = targetAutomaton.states.at(4).get();
  // Transitions from loc2
  targetAutomaton.states.at(2)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(2)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(2)->next['c'].at(0).target = targetAutomaton.states.at(3).get();
  targetAutomaton.states.at(2)->next['c'].at(0).guard = {learnta::ConstraintMaker(1) >= scale,
                                                         learnta::ConstraintMaker(1) <= scale};
  targetAutomaton.states.at(2)->next['c'].at(1).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(2)->next['c'].at(1).guard = {learnta::ConstraintMaker(1) > scale};
  targetAutomaton.states.at(2)->next['c'].at(2).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(2)->next['c'].at(2).guard = {learnta::ConstraintMaker(1) < scale};
  // Transitions from the accepting state
  targetAutomaton.states.at(3)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(3)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(3)->next['c'].at(0).target = targetAutomaton.states.at(4).get();
  // Transitions from the sink state
  targetAutomaton.states.at(4)->next['a'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(4)->next['b'].at(0).target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(4)->next['c'].at(0).target = targetAutomaton.states.at(4).get();

  targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
  targetAutomaton.maxConstraints.resize(2);
  targetAutomaton.maxConstraints[0] = scale;
  targetAutomaton.maxConstraints[1] = scale;

  // Generate the target DTA
  targetAutomaton.simplifyStrong();
  targetAutomaton.simplifyWithZones();
  BOOST_LOG_TRIVIAL(info) << "targetAutomaton:\n" << targetAutomaton;
  complementTargetAutomaton = targetAutomaton.complement({'a', 'b', 'c'});
  complementTargetAutomaton.simplifyStrong();
  complementTargetAutomaton.simplifyWithZones();
  BOOST_LOG_TRIVIAL(info) << "complementTargetAutomaton:\n" << complementTargetAutomaton;

  // Construct the learner
  const std::vector<Alphabet> alphabet = {'a', 'b', 'c'};
  auto sul = std::unique_ptr<learnta::SUL>(new learnta::TimedAutomatonRunner(targetAutomaton));
  auto memOracle = std::make_unique<learnta::SymbolicMembershipOracle>(std::move(sul));
  auto eqOracle = std::make_unique<learnta::EquivalenceOracleChain>();
  auto eqOracleByTest = std::make_unique<learnta::EquivalenceOracleByTest>(targetAutomaton);
  eqOracleByTest->push_back(learnta::TimedWord{"bc", {1.0, 0.0, 0.0}});
  eqOracle->push_back(std::unique_ptr<learnta::EquivalenceOracle>(
          new learnta::ComplementTimedAutomataEquivalenceOracle(targetAutomaton, complementTargetAutomaton, alphabet)));
  // eqOracle->push_back(std::move(eqOracleByTest));
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
    run(1);
  } else {
    for (int i = 1; i < argc; ++i) {
      std::cout << "Use scale = " << argv[i] << std::endl;
      run(atoi(argv[i]));
    }
  }

  return 0;
}

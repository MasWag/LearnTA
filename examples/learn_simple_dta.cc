/**
 * @author Masaki Waga
 * @date 2022/03/27.
 */

#include <iostream>

#include "timed_automaton.hh"
#include "learner.hh"
#include "timed_automata_equivalence_oracle.hh"
#include "timed_automaton_runner.hh"
#include "experiment_runner.hh"

void run(int scale) {
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  // Generate the target DTA
  targetAutomaton.states.resize(2);
  targetAutomaton.states.at(0) = std::make_shared<learnta::TAState>(true);
  targetAutomaton.states.at(0)->next['a'].resize(2);
  targetAutomaton.states.at(1) = std::make_shared<learnta::TAState>(false);
  targetAutomaton.states.at(1)->next['a'].resize(2);
  // Transitions from loc0
  targetAutomaton.states.at(0)->next['a'].at(0).target = targetAutomaton.states.at(0).get();
  targetAutomaton.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < scale};
  targetAutomaton.states.at(0)->next['a'].at(1).target = targetAutomaton.states.at(1).get();
  targetAutomaton.states.at(0)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) >= scale};
  targetAutomaton.states.at(0)->next['a'].at(1).resetVars.emplace_back(0, 0.0);
  // Transitions from loc1
  targetAutomaton.states.at(1)->next['a'].at(0).target = targetAutomaton.states.at(0).get();
  targetAutomaton.states.at(1)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= scale};
  targetAutomaton.states.at(1)->next['a'].at(1).target = targetAutomaton.states.at(1).get();
  targetAutomaton.states.at(1)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) > scale};

  targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
  targetAutomaton.maxConstraints.resize(1);
  targetAutomaton.maxConstraints[0] = scale;

  // Generate the complement of the target DTA
  complementTargetAutomaton.states.resize(2);
  complementTargetAutomaton.states.at(0) = std::make_shared<learnta::TAState>(false);
  complementTargetAutomaton.states.at(0)->next['a'].resize(2);
  complementTargetAutomaton.states.at(1) = std::make_shared<learnta::TAState>(true);
  complementTargetAutomaton.states.at(1)->next['a'].resize(2);
  // Transitions from loc0
  complementTargetAutomaton.states.at(0)->next['a'].at(0).target = complementTargetAutomaton.states.at(0).get();
  complementTargetAutomaton.states.at(0)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) < scale};
  complementTargetAutomaton.states.at(0)->next['a'].at(1).target = complementTargetAutomaton.states.at(1).get();
  complementTargetAutomaton.states.at(0)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) >= scale};
  complementTargetAutomaton.states.at(0)->next['a'].at(1).resetVars.emplace_back(0, 0.0);
  // Transitions from loc1
  complementTargetAutomaton.states.at(1)->next['a'].at(0).target = complementTargetAutomaton.states.at(0).get();
  complementTargetAutomaton.states.at(1)->next['a'].at(0).guard = {learnta::ConstraintMaker(0) <= scale};
  complementTargetAutomaton.states.at(1)->next['a'].at(1).target = complementTargetAutomaton.states.at(1).get();
  complementTargetAutomaton.states.at(1)->next['a'].at(1).guard = {learnta::ConstraintMaker(0) > scale};

  complementTargetAutomaton.initialStates.push_back(complementTargetAutomaton.states.at(0));
  complementTargetAutomaton.maxConstraints.resize(1);
  complementTargetAutomaton.maxConstraints[0] = scale;

  targetAutomaton.simplifyStrong();
  targetAutomaton.simplifyWithZones();
  BOOST_LOG_TRIVIAL(info) << "targetAutomaton:\n" << targetAutomaton;
  complementTargetAutomaton.simplifyStrong();
  complementTargetAutomaton.simplifyWithZones();
  BOOST_LOG_TRIVIAL(info) << "complementTargetAutomaton:\n" << complementTargetAutomaton;

  // Execute the learning
  const std::vector<learnta::Alphabet> alphabet = {'a'};
  learnta::ExperimentRunner runner {alphabet, targetAutomaton};
  runner.run();
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

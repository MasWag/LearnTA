/**
 * @brief An example to learn the Light DTA in [APT'20]
 * @author Masaki Waga
 * @date 2022/03/28.
 *
 * - [APT'20]: Aichernig, Bernhard K., Andrea Pferscher, and Martin Tappler. "From passive to active: learning timed automata efficiently." NASA Formal Methods Symposium. Springer, Cham, 2020.
 *
 * Encoding of the events are as follows
 * - p: press
 * - r: release
 * - s: starthold
 * - e: endhold
 * - t: touch
 */

#include <iostream>

#include "timed_automaton.hh"
#include "learner.hh"
#include "timed_automata_equivalence_oracle.hh"
#include "timed_automaton_runner.hh"
#include "experiment_runner.hh"

void run(int scale) {
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;
  const std::vector<learnta::Alphabet> alphabet = {'p', 'r', 's', 'e', 't'};

  // Generate the target DTA
  targetAutomaton.states.resize(6);
  for (int i = 0; i < 5; ++i) {
    targetAutomaton.states.at(i) = std::make_shared<learnta::TAState>(true);
  }
  targetAutomaton.states.at(5) = std::make_shared<learnta::TAState>(false);

  // Transitions
  targetAutomaton.states.at(0)->next['p'].emplace_back();
  targetAutomaton.states.at(0)->next['p'].back().target = targetAutomaton.states.at(1).get();
  targetAutomaton.states.at(0)->next['p'].back().resetVars.emplace_back(0, 0.0);

  targetAutomaton.states.at(1)->next['r'].emplace_back();
  targetAutomaton.states.at(1)->next['r'].back().target = targetAutomaton.states.at(0).get();
  targetAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) < scale};
  targetAutomaton.states.at(1)->next['r'].emplace_back();
  targetAutomaton.states.at(1)->next['r'].back().target = targetAutomaton.states.at(4).get();
  targetAutomaton.states.at(1)->next['r'].back().guard = {learnta::ConstraintMaker(0) >= scale};

  targetAutomaton.states.at(1)->next['s'].emplace_back();
  targetAutomaton.states.at(1)->next['s'].back().target = targetAutomaton.states.at(2).get();
  targetAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(0) >= 2 * scale};
  targetAutomaton.states.at(1)->next['s'].emplace_back();
  targetAutomaton.states.at(1)->next['s'].back().target = targetAutomaton.states.at(5).get();
  targetAutomaton.states.at(1)->next['s'].back().guard = {learnta::ConstraintMaker(0) < 2 * scale};

  targetAutomaton.states.at(2)->next['r'].emplace_back();
  targetAutomaton.states.at(2)->next['r'].back().target = targetAutomaton.states.at(3).get();

  targetAutomaton.states.at(3)->next['e'].emplace_back();
  targetAutomaton.states.at(3)->next['e'].back().target = targetAutomaton.states.at(0).get();

  targetAutomaton.states.at(4)->next['t'].emplace_back();
  targetAutomaton.states.at(4)->next['t'].back().target = targetAutomaton.states.at(0).get();

  targetAutomaton.initialStates.push_back(targetAutomaton.states.at(0));
  targetAutomaton.maxConstraints.resize(1);
  targetAutomaton.maxConstraints[0] = 2 * scale;

  // Execute the learning
  learnta::ExperimentRunner runner {alphabet, targetAutomaton};
  runner.run();
}

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  std::cout << "Usage: " << argv[0] << " [scales]" << std::endl;
  if (argc == 1) {
    std::cout << "Use the default scale (5)" << std::endl;
    run(5);
  } else {
    for (int i = 1; i < argc; ++i) {
      std::cout << "Use scale = " << argv[i] << std::endl;
      run(atoi(argv[i]));
    }
  }

  return 0;
}

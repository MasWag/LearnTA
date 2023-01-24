/**
 * @author Masaki Waga
 * @date 2023/01/24.
 * @brief Implements the "Unbalanced Loop" benchmark, which is inspired by the unbalanced TRE in [ACM'02]
 */

#include <iostream>

#include "unbalanced_loop_fixture.hh"
#include "experiment_runner.hh"

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  if (argc < 4) {
    std::cout << "Usage: " << argv[0] << " [states] [clocks] [scales]" << std::endl;
  } else {
    int states = atoi(argv[1]);
    int clocks = atoi(argv[2]);
    int scales = atoi(argv[3]);

    BOOST_LOG_TRIVIAL(info) << "Start learning with the following parameters\n"
                            << "states: " << states << "\n"
                            << "clocks: " << clocks << "\n"
                            << "scales: " << scales;
    UnbalancedLoopFixture fixture{states, clocks, scales};
    learnta::ExperimentRunner runner{fixture.alphabet, fixture.targetAutomaton};
    runner.run();
  }

  return 0;
}

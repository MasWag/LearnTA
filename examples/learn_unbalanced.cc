/**
 * @author Masaki Waga
 * @date 2022/06/26.
 * @brief Implements the "Unbalanced" benchmark, which is inspired by the unbalanced TRE in [ACM'02]
 */

#include <iostream>

#include "unbalanced_fixture.hh"
#include "experiment_runner.hh"

void run(int scale) {
  UnbalancedFixture fixture{scale};
  learnta::ExperimentRunner runner{fixture.alphabet, fixture.targetAutomaton};
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

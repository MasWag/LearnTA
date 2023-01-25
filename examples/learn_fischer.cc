/**
 * @author Masaki Waga
 * @date 2023/01/25.
 * @brief Implements the Fischer benchmark, originally from [AL'94]
 */

#include "fischer_fixture.hh"
#include "experiment_runner.hh"

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  BOOST_LOG_TRIVIAL(info) << "Usage: " << argv[0] << " [scales]";
  if (argc == 1) {
    BOOST_LOG_TRIVIAL(info) << "Use the default scale: 10";
    FischerFixture fixture{10};
    learnta::ExperimentRunner runner{fixture.alphabet, fixture.targetAutomaton};
    runner.run();
  } else {
    for (int i = 1; i < argc; ++i) {
      BOOST_LOG_TRIVIAL(info) << "Use scale = " << argv[i];
      FischerFixture fixture{atoi(argv[i])};
      learnta::ExperimentRunner runner{fixture.alphabet, fixture.targetAutomaton};
      runner.run();
    }
  }

  return 0;
}

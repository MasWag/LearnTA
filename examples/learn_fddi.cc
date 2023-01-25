/**
 * @author Masaki Waga
 * @date 2023/01/24.
 * @brief Implements the FDDI benchmark, originally from [DOTY'96]
 */

#include "fddi_fixture.hh"
#include "experiment_runner.hh"

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  BOOST_LOG_TRIVIAL(info) << "Usage: " << argv[0] << " [scales]";
  if (argc == 1) {
    BOOST_LOG_TRIVIAL(info) << "Use the default scale: 20";
    FDDIFixture fixture{20};
    learnta::ExperimentRunner runner{fixture.alphabet, fixture.targetAutomaton};
    runner.run();
  } else {
    for (int i = 1; i < argc; ++i) {
      BOOST_LOG_TRIVIAL(info) << "Use scale = " << argv[i];
      FDDIFixture fixture{atoi(argv[i])};
      learnta::ExperimentRunner runner{fixture.alphabet, fixture.targetAutomaton};
      runner.run();
    }
  }

  return 0;
}

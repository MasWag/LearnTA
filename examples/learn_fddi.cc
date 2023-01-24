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

  FDDIFixture fixture{};
  learnta::ExperimentRunner runner{fixture.alphabet, fixture.targetAutomaton};
  runner.run();

  return 0;
}

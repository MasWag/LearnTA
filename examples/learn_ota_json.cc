/**
 * @author Masaki Waga
 * @date 2022/09/16.
 * @brief Learns a one-clock DTA in the json format of https://github.com/Leslieaj/OTALearning
 */

#include <iostream>

#include "ota_json_parser.hh"
#include "experiment_runner.hh"

void run(const std::string &jsonPath) {
  learnta::OtaJsonParser parser{jsonPath};
  learnta::ExperimentRunner runner{parser.getAlphabet(), parser.getTarget() };
  runner.run();
}

int main(int argc, const char *argv[]) {
#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  std::cout << "Usage: " << argv[0] << " [json path]" << std::endl;
  if (argc <= 1) {
    std::cout << "json file is not specified" << std::endl;
    return 1;
  } else {
      run(argv[1]);
  }

  return 0;
}

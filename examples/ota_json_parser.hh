/**
 * @author Masaki Waga
 * @date 2022/09/16.
 */

#pragma once

#include <stdexcept>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include "timed_automaton.hh"


namespace learnta {

/*!
 * @brief Parser of one clock TAs in the json format of https://github.com/Leslieaj/OTALearning
 */
  class OtaJsonParser {
  private:
    std::vector<Alphabet> alphabet;
    TimedAutomaton target;
    inline static void validateLabel(const std::string &label) {
      if (label.size() != 1) {
        throw std::invalid_argument("Invalid alphabet: " + label);
      }
    }
  public:
    explicit OtaJsonParser(const std::string &jsonPath) {
      target.maxConstraints = {0};

      boost::property_tree::ptree pt;
      boost::property_tree::read_json(jsonPath, pt);

      // Construct the alphabet
      BOOST_FOREACH (const auto &child, pt.get_child("sigma")) {
        learnta::OtaJsonParser::validateLabel(child.second.data());
        this->alphabet.push_back(child.second.data().front());
      }

      // Construct states
      std::unordered_map<std::string, std::shared_ptr<TAState>> states;
      const auto acceptNode = pt.get_child("accept");
      BOOST_FOREACH (const auto &child, pt.get_child("l")) {
              const std::string &stateName = child.second.data();
              bool isMatch = false;
              BOOST_FOREACH(const auto &acceptedState, acceptNode) {
                      if (acceptedState.second.data() == stateName) {
                        isMatch = true;
                        break;
                      }
                    }
              states[stateName] = std::make_shared<TAState>(isMatch);
            }

      // Construct the states
      target.states.reserve(states.size());
      std::transform(states.begin(), states.end(), std::back_inserter(target.states), [](const auto &state) {
        return state.second;
      });

      // Construct the initial states
      target.initialStates = {states.at(pt.get_child("init").data())};

      // Construct the transitions
      BOOST_FOREACH (const auto &child, pt.get_child("tran")) {
        auto it = child.second.begin();
        const std::string sourceName = it->second.data();
        it = boost::next(it);
        learnta::OtaJsonParser::validateLabel(it->second.data());
        const Alphabet label = it->second.data().front();
        it = boost::next(it);
        const std::string range = it->second.data();
        it = boost::next(it);
        const bool reset = it->second.data() == "r";
        it = boost::next(it);
        const std::string targetName = it->second.data();

        // Make guard
        std::vector<Constraint> guard;
        const auto pos = range.find(',');
        // Make the lower bound
        const std::string lowerBoundString = range.substr(0, pos);
        if (lowerBoundString != "[0") {
          int constant = atoi(lowerBoundString.substr(1).c_str());
          target.maxConstraints.at(0) = std::max(target.maxConstraints.at(0), constant);
          if (lowerBoundString.front() == '[') {
            // Closed
            guard.push_back(ConstraintMaker(0) >= constant);
          } else {
            // Open
            guard.push_back(ConstraintMaker(0) > constant);
          }
        }
        // Make the upper bound
        std::string upperBoundString = range.substr(pos + 1);
        if (upperBoundString != "+)") {
          if (upperBoundString.back() == ']') {
            upperBoundString.pop_back();
            int constant = atoi(upperBoundString.c_str());
            target.maxConstraints.at(0) = std::max(target.maxConstraints.at(0), constant);
            // Closed
            guard.push_back(ConstraintMaker(0) <= atoi(upperBoundString.c_str()));
          } else {
            int constant = atoi(upperBoundString.c_str());
            target.maxConstraints.at(0) = std::max(target.maxConstraints.at(0), constant);
            // Open
            guard.push_back(ConstraintMaker(0) < atoi(upperBoundString.c_str()));
          }
        }
        const auto sourceState = states.at(sourceName);
        const auto targetState = states.at(targetName).get();
        TATransition::Resets resetVariables;
        if (reset) {
          resetVariables.emplace_back(0, 0.0);
        }
        if (sourceState->next.find(label) == sourceState->next.end()) {
          sourceState->next[label] = {TATransition{targetState, resetVariables, guard}};
        } else {
          sourceState->next[label].emplace_back(targetState, resetVariables, guard);
        }
      }

      if (!target.deterministic()) {
        throw std::invalid_argument("Nondeterministic TA is given");
      }
    }

    [[nodiscard]] const std::vector<Alphabet> &getAlphabet() const {
      return alphabet;
    }

    [[nodiscard]] const TimedAutomaton &getTarget() const {
      return target;
    }
  };
}
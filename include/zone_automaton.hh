#pragma once

#include <stack>
#include <unordered_set>
#include <utility>

#include "timed_automaton.hh"
#include "zone.hh"
#include "timed_word.hh"
#include "symbolic_run.hh"
#include "zone_automaton_state.hh"

namespace learnta {
  /*!
   * @brief A Zone automaton
   */
  struct ZoneAutomaton : public Automaton<ZAState> {
    /*!
     * @brief Sample a timed word in this zone automaton
     *
     * We use the reconstruction algorithm in [Andre+, NFM'22]
     */
    [[nodiscard]] std::optional<TimedWord> sample() const {
      std::vector<SymbolicRun> currentStates;
      currentStates.reserve(initialStates.size());
      std::transform(initialStates.begin(), initialStates.end(), std::back_inserter(currentStates),
                     [](const auto &initialState) {
                       return SymbolicRun{initialState};
                     });
      std::unordered_set<std::shared_ptr<ZAState>> visited = {initialStates.begin(), initialStates.end()};

      while (!currentStates.empty()) {
        std::vector<SymbolicRun> nextStates;
        for (const auto &run: currentStates) {
          if (run.back()->isMatch) {
            // run is a positive run
            return std::make_optional(run.reconstructWord());
          }
          for (int action = 0; action < CHAR_MAX; ++action) {
            const auto &edges = run.back()->next[action];
            for (const auto &edge: edges) {
              auto transition = edge.first;
              auto target = edge.second.lock();
              if (target && visited.find(target) == visited.end()) {
                // We have not visited the state
                auto newRun = run;
                newRun.push_back(transition, action, target);
                nextStates.push_back(newRun);
                visited.insert(target);
              }
            }
          }
        }
        currentStates = std::move(nextStates);
      }

      return std::nullopt;
    }
  };
}
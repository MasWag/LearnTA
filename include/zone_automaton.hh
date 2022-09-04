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
            auto wordOpt = run.reconstructWord();
            if (wordOpt) {
              return wordOpt;
            }
          }
          for (int action = 0; action < CHAR_MAX; ++action) {
            const auto &edges = run.back()->next[action];
            for (const auto &edge: edges) {
              auto transition = edge.first;
              auto target = edge.second.lock();
              if (target && visited.find(target) == visited.end()) {
                // We have not visited the state
                auto newRun = run;
                newRun.push_back(transition, static_cast<char>(action), target);
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

    std::optional<TimedWord> sampleMemo;
    std::optional<TimedWord> sampleWithMemo() {
      if (sampleMemo) {
        return sampleMemo;
      } else {
        sampleMemo = sample();
        return sampleMemo;
      }
    }

    /*!
     * @brief Remove dead states, i.e., unreachable to any of the accepting states
     */
    void removeDeadStates() {
      std::unordered_map<std::shared_ptr<ZAState>, std::unordered_set<std::shared_ptr<ZAState>>> backwardEdges;
      for (const auto &state: this->states) {
        for (const auto &transitions: state->next) {
          for (const auto &[transition, target]: transitions) {
            auto it = backwardEdges.find(target.lock());
            if (it == backwardEdges.end()) {
              backwardEdges[target.lock()] = {state};
            } else {
              backwardEdges.at(target.lock()).insert(state);
            }
          }
        }
      }

      // The states reachable to an accepting state
      std::unordered_set<std::shared_ptr<ZAState>> liveStates;
      std::queue<std::shared_ptr<ZAState>> newLiveStates;
      for (const auto &state: this->states) {
        if (state->isMatch) {
          liveStates.insert(state);
          newLiveStates.push(state);
        }
      }
      while (!newLiveStates.empty()) {
        const auto newLiveState = newLiveStates.front();
        newLiveStates.pop();
        if (backwardEdges.find(newLiveState) != backwardEdges.end()) {
          for (const auto &backwardState: backwardEdges.at(newLiveState)) {
            if (liveStates.find(backwardState) == liveStates.end()) {
              liveStates.insert(backwardState);
              newLiveStates.push(backwardState);
            }
          }
        }
      }

      if (liveStates.size() != this->stateSize()) {
        // Remove dead states if exists
        BOOST_LOG_TRIVIAL(info) << "There are " << this->stateSize() - liveStates.size() << " dead states in the zone graph";
        this->states.erase(std::remove_if(this->states.begin(), this->states.end(), [&](const auto &state) {
          return liveStates.find(state) == liveStates.end();
        }), this->states.end());
        this->initialStates.erase(
                std::remove_if(this->initialStates.begin(), this->initialStates.end(), [&](const auto &state) {
                  return liveStates.find(state) == liveStates.end();
                }), this->initialStates.end());
        for (const auto &state: this->states) {
          for (auto &transitions: state->next) {
            transitions.erase(std::remove_if(transitions.begin(), transitions.end(), [&](const auto &pair) {
              const auto &[transition, target] = pair;
              return liveStates.find(target.lock()) == liveStates.end();
            }), transitions.end());
          }
        }
      }
    }
  };
}
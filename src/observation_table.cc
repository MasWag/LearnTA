/**
 * @author Masaki Waga
 * @date 2022/12/23.
 */

#include <boost/unordered_map.hpp>

#include "observation_table.hh"

namespace learnta {
  /*!
   * @brief Handle inactive clock variables in the DTA construction
   *
   * A clock variable is deactivated if its value may not be the same as the one in the corresponding reduction in the recognizable timed language
   * This happens when we use (u, Λ, u', Λ', R) such that (w, w') ∈ (u, Λ, u', Λ', R) ∧ (w, w'') ∈ (u, Λ, u', Λ', R) does not imply w' = w''
   * By definition of recognizable timed languages, such w, w', and w'' are equivalent with respect to any extension
   *
   * A clock variable c is /deactivated/ by a transition (l, g, a, ρ, l') if we have one of the following
   * - ρ(c) is a non-integer constant
   * - there is an inactive clock c' such that ρ(c) == c'
   *
   * A clock variable c is activated by a transition (l, g, a, ρ, l') such that
   * - ρ(c) is an integer constant
   * - there is an active clock c' such that ρ(c) == c'
   */
  void ObservationTable::handleInactiveClocks(std::vector<std::shared_ptr<TAState>> &states) {
    const auto originalStates = states;
    boost::unordered_map<std::pair<TAState*, std::vector<ClockVariables>>, std::shared_ptr<TAState>> map;
    std::queue<std::pair<TAState*, std::vector<ClockVariables>>> inactiveQueue;
    // Initialize the inactive Queue
    for (const auto& state: originalStates) {
      map[std::make_pair(state.get(), std::vector<ClockVariables>{})] = state;
    }
    for (const auto& state: originalStates) {
      for (auto &[action, transitions]: state->next) {
        for (auto &transition: transitions) {
          std::vector<ClockVariables> inactiveClocks;
          for (const auto &[clock, value]: transition.resetVars) {
            if (value.index() == 0 && std::get<double>(value) != int(std::get<double>(value))) {
              inactiveClocks.push_back(clock);
            }
          }
          std::sort(inactiveClocks.begin(), inactiveClocks.end());
          inactiveClocks.erase(std::unique(inactiveClocks.begin(), inactiveClocks.end()), inactiveClocks.end());
          auto it = map.find(std::make_pair(transition.target, inactiveClocks));
          if (it == map.end()) {
            states.push_back(std::make_shared<TAState>(transition.target->isMatch,
                                                       transition.target->next));
            map[std::make_pair(transition.target, inactiveClocks)] = states.back();
            inactiveQueue.emplace(transition.target, inactiveClocks);
          }
          transition.target = map.at(std::make_pair(transition.target, inactiveClocks)).get();
        }
      }
    }
    // TODO: Handling of inactive clocks is still wrong!!
    while (!inactiveQueue.empty()) {
      const auto [originalState, inactiveClocks] = inactiveQueue.front();
      const auto state = map.at(std::make_pair(originalState, inactiveClocks));
      inactiveQueue.pop();
      for (auto &[action, transitions]: state->next) {
        for (auto &transition: transitions) {
          for (auto it = transition.guard.begin(); it != transition.guard.end();) {
            // remove inactive clock variables
            if (std::find(inactiveClocks.begin(), inactiveClocks.end(), it->x) != inactiveClocks.end()) {
              it = transition.guard.erase(it);
            } else {
              ++it;
            }
          }
          std::vector<ClockVariables> nextInactiveClocks;
          if (transition.guard.empty()) {
            nextInactiveClocks = inactiveClocks;
          }
          for (const auto &[clock, value]: transition.resetVars) {
            if (value.index() == 0 && std::get<double>(value) != int (std::get<double>(value))) {
              nextInactiveClocks.push_back(clock);
            }
          }
          for (const auto &inactiveClock: inactiveClocks) {
            // the clock is still inactive if it is not updated
            auto it = std::find_if(transition.resetVars.begin(), transition.resetVars.end(), [=] (const auto &p) {
              return p.first == inactiveClock;
            });
            if (it == transition.resetVars.end()) {
              nextInactiveClocks.push_back(inactiveClock);
            }
            // c' is deactivated if it is updated to the current inactive clock
            it = std::find_if(transition.resetVars.begin(), transition.resetVars.end(), [=] (const auto &p) {
              return p.second.index() == 1 && std::get<ClockVariables>(p.second) == inactiveClock;
            });
            if (it != transition.resetVars.end()) {
              nextInactiveClocks.push_back(it->first);
            }
          }
          std::sort(nextInactiveClocks.begin(), nextInactiveClocks.end());
          nextInactiveClocks.erase(std::unique(nextInactiveClocks.begin(), nextInactiveClocks.end()), nextInactiveClocks.end());
          auto it = map.find(std::make_pair(transition.target, nextInactiveClocks));
          if (it == map.end()) {
            states.push_back(std::make_shared<TAState>(transition.target->isMatch));
            map[std::make_pair(transition.target, nextInactiveClocks)] = states.back();
            inactiveQueue.emplace(transition.target, nextInactiveClocks);
          }
          transition.target = map.at(std::make_pair(transition.target, nextInactiveClocks)).get();
        }
      }
    }
  }

  namespace InternalSplitStates {
    using PreciseClocks = std::vector<ClockVariables>;
    using State = TAState*;
    // A pair of the original state and the precise clocks
    using EnhancedState = std::pair<State, PreciseClocks>;
    std::unordered_set<ClockVariables> asSet(const std::vector<ClockVariables> &vector) {
      return std::unordered_set<ClockVariables>{vector.begin(), vector.end()};
    }
    PreciseClocks asPreciseClocks(const std::unordered_set<ClockVariables> &set) {
      auto vector = PreciseClocks{set.begin(), set.end()};
      std::sort(vector.begin(), vector.end());
      return vector;
    }
    struct StateMap {
      std::vector<std::shared_ptr<TAState>> states;
      const std::vector<std::shared_ptr<TAState>> originalStates;
      std::shared_ptr<TAState> initialState;
      boost::unordered_map<EnhancedState, State> forwardMap;
      boost::unordered_map<State, EnhancedState> reverseMap;

      StateMap(const std::vector<std::shared_ptr<TAState>> &states, const std::shared_ptr<TAState> &initialState) :originalStates(states) {
        for (const auto& state: states) {
          this->states.push_back(std::make_shared<TAState>(*state));
          const auto clockSize = NeighborConditions::computeClockSize(state.get());
          std::vector<ClockVariables> preciseClocks;
          preciseClocks.resize(clockSize);
          std::iota(preciseClocks.begin(), preciseClocks.end(), 0);

          this->add(EnhancedState{state.get(), PreciseClocks{preciseClocks.begin(), preciseClocks.end()}},
                    this->states.back().get());
          if (state == initialState) {
            this->initialState = this->states.back();
          }
        }
        for (auto& state: this->states) {
          for (auto &[action, transitions]: state->next) {
            for (auto &transition: transitions) {
              transition.target = this->fromOriginalState(transition.target);
            }
          }
        }
        assert(this->forwardMap.size() == this->states.size());
        assert(this->reverseMap.size() == this->states.size());
      }

      [[nodiscard]] State map(const EnhancedState &state) const {
        auto it = forwardMap.find(state);
        assert(it != forwardMap.end());

        return it->second;
      }

      [[nodiscard]] EnhancedState map(const State &state) const {
        auto it = reverseMap.find(state);
        assert(it != reverseMap.end());

        return it->second;
      }

      [[nodiscard]] State toOriginalState(const State &state) const {
        return this->map(state).first;
      }

      [[nodiscard]] State fromOriginalState(const State &state) const {
        assert(std::find_if(originalStates.begin(), originalStates.end(), [&] (const auto &s) {
          return s.get() != state;
        }) != originalStates.end());
        auto it = std::find_if(forwardMap.begin(), forwardMap.end(), [&] (const auto &pair) {
          return pair.first.first == state;
        });
        assert(it != forwardMap.end());

        return it->second;
      }

      void add(const EnhancedState &state, const State &newState) {
        assert(std::find_if(originalStates.begin(), originalStates.end(), [&] (const auto &s) {
          return s.get() == state.first;
        }) != originalStates.end());
        forwardMap[state] = newState;
        reverseMap[newState] = state;
      }

      State make(const EnhancedState &state) {
        BOOST_LOG_TRIVIAL(debug) << "StateMap: new state is created";
        assert(std::find_if(originalStates.begin(), originalStates.end(), [&] (const auto &s) {
          return s.get() == state.first;
        }) != originalStates.end());
        auto it = this->forwardMap.find(state);
        if (it == this->forwardMap.end()) {
          states.push_back(std::make_shared<TAState>(state.first->isMatch));
          add(state, states.back().get());
        }

        return map(state);
      }
    };
  }

  void ObservationTable::splitStates(std::vector<std::shared_ptr<TAState>> &originalStates,
                                     std::shared_ptr<TAState> &initialState,
                                     const std::vector<TAState *> &needSplit) {
    using namespace InternalSplitStates;
    if (needSplit.empty()) {
      return;
    }

    // Initialize stateMap
    StateMap stateMap{originalStates, initialState};
    // Set of the visited states
    boost::unordered_set<EnhancedState> visitedStates;
    const auto isVisited = [&] (const auto& state) {
      return visitedStates.find(state) != visitedStates.end();
    };
#ifdef DEBUG
    const auto inOriginalStates = [&] (const auto& state) {
      return std::find_if(originalStates.begin(), originalStates.end(), [&] (const auto& s) {
        return s.get() == state;
      }) != originalStates.end();
    };
#endif
    // Queue of the states to explore
    std::queue<EnhancedState> statesToVisit;
    statesToVisit.emplace(initialState.get(), PreciseClocks{0});

    // The main loop. We conduct BFS over the DTA
    while (!statesToVisit.empty()) {
      const EnhancedState enhancedState = statesToVisit.front();
      const auto &[originalState, preciseClocks] = enhancedState;
      assert(inOriginalStates(originalState));
      const auto state = stateMap.map(enhancedState);
      assert(!inOriginalStates(state));
      statesToVisit.pop();
      if (isVisited(enhancedState)) {
        continue;
      }
      visitedStates.insert(enhancedState);
      originalState->mergeNondeterministicBranching(asSet(preciseClocks));
      for (auto &[action, transitions]: state->next) {
        for (auto &transition: transitions) {
          const auto nextPreciseClocks = NeighborConditions::preciseClocksAfterReset(asSet(preciseClocks), transition);
          const auto targetAsOriginal = stateMap.toOriginalState(transition.target);
          assert(inOriginalStates(targetAsOriginal));
          const EnhancedState nextEnhancedState{targetAsOriginal, asPreciseClocks(nextPreciseClocks)};
          if (!isVisited(nextEnhancedState)) {
            statesToVisit.push(nextEnhancedState);
            assert(inOriginalStates(nextEnhancedState.first));
            auto newState = stateMap.make(nextEnhancedState);
            newState->next = transition.target->next;
            newState->mergeNondeterministicBranching(nextPreciseClocks);
          }
          // We update the target location
          transition.target = stateMap.map(nextEnhancedState);
        }
      }
    }

    originalStates = stateMap.states;
    initialState = stateMap.initialState;
  }
}

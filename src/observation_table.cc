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
}
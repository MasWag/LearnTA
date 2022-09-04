#include <numeric>
#include <utility>

#include "../include/ta2za.hh"

namespace learnta {

/*!
  @brief Generate a zone automaton from a timed automaton
  @tparam NVar the number of variable in TA

  TA to ZA adds states with BFS. Initial configuration is the initial states of
  ZA. The ZA contain only the states reachable from initial states.
 */
  void ta2za(const TimedAutomaton &TA, ZoneAutomaton &ZA, bool quickReturn) {
    const std::size_t clockSize = TA.clockSize();
    Zone initialZone = Zone::zero(clockSize + 1);

    //! number of states of ZA
    if (clockSize > 0) {
      initialZone.M = Bounds{
              *std::max_element(TA.maxConstraints.begin(), TA.maxConstraints.end()),
              true};
      initialZone.maxConstraints.resize(TA.maxConstraints.size());
      for (int i = 0; i < initialZone.maxConstraints.size(); ++i) {
        initialZone.maxConstraints.at(i) = TA.maxConstraints.at(i);
      }
    } else {
      initialZone.M = Bounds(0, true);
    }

    /*!
      @brief Make initial state, that is Current configuration of BFS
    */
    std::deque<std::shared_ptr<ZAState>> newStates;
    ZA.initialStates.resize(TA.initialStates.size());
    std::transform(TA.initialStates.begin(), TA.initialStates.end(), ZA.initialStates.begin(),
                   [&initialZone](const auto &state) {
                     return std::make_shared<ZAState>(state.get(), initialZone);
                   });
    ZA.states.resize(ZA.initialStates.size());
    std::copy(ZA.initialStates.begin(), ZA.initialStates.end(), ZA.states.begin());
    std::copy(ZA.initialStates.begin(), ZA.initialStates.end(), std::back_inserter(newStates));

    /*!
      @brief translater from TAState and Zone to its corresponding state in ZA.

      The type is like this.
      (TAState,Zone) -> ZAState
    */
    boost::unordered_map<std::pair<TAState *, Zone>, std::shared_ptr<ZAState>> zaMap;
    for (const auto &state: ZA.initialStates) {
      zaMap[std::make_pair(state->taState, state->zone)] = state;
    }
    while (!newStates.empty()) {
      const auto zaState = newStates.front();
      newStates.pop_front();
      TAState *taState = zaState->taState;
      Zone nowZone = zaState->zone;
      nowZone.elapse();
      for (const auto &[c, edges]: taState->next) {
        for (const auto &edge: edges) {
          Zone nextZone = nowZone;
          auto nextState = edge.target;
          if (!nextState) {
            continue;
          }
          nextZone.tighten(edge.guard);

          if (nextZone) {
            nextZone.applyResets(edge.resetVars);
            nextZone.canonize();
            if (!nextZone.isSatisfiable()) {
              continue;
            }
            nextZone.value.diagonal().fill(Bounds{0, true});
            if (!quickReturn) {
              nextZone.extrapolate();
              nextZone.value.diagonal().fill(Bounds{0, true});
            }

            const auto targetStateInZA = std::find_if(zaMap.begin(), zaMap.end(), [&] (const auto &pair) {
             // if (quickReturn) {
                // Use inclusion for state merging
                return pair.first.first == nextState && pair.first.second.includes(nextZone);
             // } else {
                // Use equality
              //  return pair.first.first == nextState && pair.first.second == nextZone;
              //}
            });

            // targetStateInZA is already added
            if (targetStateInZA != zaMap.end()) {
              zaState->next[c].emplace_back(edge, targetStateInZA->second);
            } else {
              // targetStateInZA is new
              if (quickReturn) {
                nextZone.extrapolate();
                nextZone.canonize();
                nextZone.value.diagonal().fill(Bounds{0, true});
              }
              ZA.states.push_back(std::make_shared<ZAState>(nextState, nextZone));
              zaState->next[c].emplace_back(edge, ZA.states.back());

              newStates.push_back(ZA.states.back());
              zaMap[std::make_pair(ZA.states.back()->taState, ZA.states.back()->zone)] = ZA.states.back();
            }
            // We shortcut the zone construction once we reach an accepting state
            if (nextState->isMatch) {
              if (quickReturn && ZA.sampleWithMemo()) {
                return;
              }
            }
          }
        }
      }
    }
  }
}

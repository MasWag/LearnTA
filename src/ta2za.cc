#include <cstdlib>
#include <numeric>
#include <utility>
#include <execution>

#include "../include/ta2za.hh"

namespace learnta {

/*!
  @brief Generate a zone automaton from a timed automaton
  @tparam NVar the number of variable in TA

  TA to ZA adds states with BFS. Initial configuration is the initial states of
  ZA. The ZA contain only the states reachable from initial states.
 */
  void ta2za(const TimedAutomaton &TA, ZoneAutomaton &ZA) {
    const std::size_t clockSize = TA.clockSize();
    Zone initialZone = Zone::zero(clockSize + 1);

    //! number of states of ZA
    if (clockSize > 0) {
      initialZone.M = Bounds{
              *std::max_element(TA.maxConstraints.begin(), TA.maxConstraints.end()),
              true};
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
    while (!newStates.empty()) {
      const auto zaState = newStates.front();
      newStates.pop_front();
      TAState *taState = zaState->taState;
      Zone nowZone = zaState->zone;
      nowZone.elapse();
      for (auto &it: taState->next) {
        const Alphabet c = it.first;
        for (const auto &edge: it.second) {
          Zone nextZone = nowZone;
          auto nextState = edge.target;
          if (!nextState) {
            continue;
          }
          for (const auto &delta: edge.guard) {
            nextZone.tighten(delta);
          }

          if (nextZone) {
            const auto nextZoneBeforeReset = nextZone;
            for (const auto &[resetVariable, updatedVariable]: edge.resetVars) {
              if (updatedVariable && resetVariable != *updatedVariable) {
                nextZone.unconstrain(resetVariable);
                nextZone.value(resetVariable + 1, *updatedVariable + 1) = Bounds{0.0, true};
                nextZone.value(*updatedVariable + 1, resetVariable + 1) = Bounds{0.0, true};
              }
            }
            nextZone.canonize();
            for (const auto &[resetVariable, updatedVariable]: edge.resetVars) {
              if (!updatedVariable) {
                nextZone.reset(resetVariable);
              }
            }
            nextZone.canonize();
            nextZone.abstractize();
            if (!nextZone.isSatisfiable()) {
              continue;
            }
            nextZone.value.diagonal().fill(Bounds{0, true});

            const auto targetStateInZA = std::find_if(zaMap.begin(), zaMap.end(), [&] (const auto &pair) {
              return pair.first.first == nextState && pair.first.second.includes(nextZone);
            });

            // targetStateInZA is already added
            if (targetStateInZA != zaMap.end()) {
              zaState->next[c].emplace_back(edge, targetStateInZA->second);
            } else {
              // targetStateInZA is new
              ZA.states.push_back(std::make_shared<ZAState>(nextState, nextZone));
              zaState->next[c].emplace_back(edge, ZA.states.back());

              newStates.push_back(ZA.states.back());
              zaMap[std::make_pair(ZA.states.back()->taState, ZA.states.back()->zone)] = ZA.states.back();
            }
            // We shortcut the zone construction once we reach an accepting state
            if (nextState->isMatch) {
              return;
            }
          }
        }
      }
    }
  }
}
#include <cstdlib>
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
  void ta2za(const TimedAutomaton &TA, ZoneAutomaton &ZA, Zone initialZone) {
    const std::size_t clockSize = TA.clockSize();
    if (initialZone.value.size() == 0) {
      initialZone = Zone::zero(clockSize + 1);
    }

    auto initialStates = TA.initialStates;
    if (!ZA.states.empty()) {
      for (auto it = initialStates.begin(); it != initialStates.end();) {
        if (std::find_if(ZA.states.begin(), ZA.states.end(),
                         [&it, &initialZone](const std::shared_ptr<ZAState>& zaState) {
                           return *zaState == std::make_pair(it->get(), initialZone);
                         }) != ZA.states.end()) {
          it = initialStates.erase(it);
        } else {
          it++;
        }
      }
    }
    if (initialStates.empty()) {
      return;
    }

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
    std::vector<std::shared_ptr<ZAState>> nextConf;
    nextConf.reserve(initialStates.size());
    ZA.states.reserve(ZA.stateSize() + initialStates.size());
    ZA.initialStates.reserve(ZA.initialStates.size() + initialStates.size());
    for (const auto &taState: initialStates) {
      ZA.states.push_back(std::make_shared<ZAState>(taState.get(), initialZone));
      ZA.initialStates.push_back(ZA.states.back());
      nextConf.push_back(ZA.states.back());
    }

    /*!
      @brief translater from TAState and Zone to its corresponding state in ZA.

      The type is like this.
      (TAState,Zone) -> ZAState
    */
    while (!nextConf.empty()) {
      std::vector<std::shared_ptr<ZAState>> currentConf = nextConf;
      nextConf.clear();
      for (const auto &conf: currentConf) {
        TAState *taState = conf->taState;
        Zone nowZone = conf->zone;
        nowZone.elapse();
        for (auto & it : taState->next) {
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

            if (nextZone.isSatisfiable()) {
              const auto nextZoneBeforeReset = nextZone;
              for (const auto &[resetVariable, updatedVariable]: edge.resetVars) {
                if (updatedVariable && resetVariable != *updatedVariable) {
                  nextZone.value.col(resetVariable + 1) = nextZoneBeforeReset.value.col(*updatedVariable + 1);
                  nextZone.value.row(resetVariable + 1) = nextZoneBeforeReset.value.row(*updatedVariable + 1);
                }
              }
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
              // nextZone state is new
              const auto targetStateInZA = std::find_if(
                      ZA.states.begin(), ZA.states.end(),
                      [&nextState, &nextZone](const std::shared_ptr<ZAState> &zaState) {
                        return *zaState == std::make_pair(nextState, nextZone);
                      });

              // targetStateInZA is already added
              if (targetStateInZA != ZA.states.end()) {
                conf->next[c].emplace_back(edge, *targetStateInZA);
              } else {
                // targetStateInZA is new
                ZA.states.push_back(std::make_shared<ZAState>(nextState, nextZone));
                conf->next[c].emplace_back(edge, ZA.states.back());

                nextConf.push_back(ZA.states.back());
              }
            }
          }
        }
      }
    }
  }
}
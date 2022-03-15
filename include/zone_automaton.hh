#pragma once

#include "timed_automaton.hh"
#include "zone.hh"
#include "timed_word.hh"
#include <stack>
#include <unordered_set>
#include <utility>

namespace learnta {
  struct ZAState {
    bool isMatch;
    // An epsilon transition is denoted by the null character (\0)
    std::array<std::vector<std::pair<TATransition, std::weak_ptr<ZAState>>>, CHAR_MAX> next;
    TAState *taState = nullptr;
    Zone zone;

    ZAState() : isMatch(false), next({}) {}

    ZAState(TAState *taState, Zone zone) : isMatch(taState->isMatch), next({}), taState(taState),
                                           zone(std::move(zone)) {}

    explicit ZAState(bool isMatch) : isMatch(isMatch), next({}) {}

    ZAState(bool isMatch, std::array<std::vector<std::pair<TATransition, std::weak_ptr<ZAState>>>, CHAR_MAX> next)
            : isMatch(isMatch), next(std::move(next)) {}

    bool operator==(const std::pair<TAState *, Zone> &pair) const {
      return pair.first == taState && pair.second == zone;
    }
  };

  struct NoEpsilonZAState {
    bool isMatch;
    std::array<std::vector<std::weak_ptr<NoEpsilonZAState>>, CHAR_MAX> next;
    std::unordered_set<std::shared_ptr<ZAState>> zaStates;

    bool operator==(const std::unordered_set<std::shared_ptr<ZAState>> &zas) const {
      return zas == zaStates;
    }
  };

//! @brief returns the set of states that is reachable from a state in the state
//! by unobservable transitions
  static inline void
  epsilonClosure(std::unordered_set<std::shared_ptr<ZAState>> &closure) {
    auto waiting =
            std::deque<std::shared_ptr<ZAState>>(closure.begin(), closure.end());
    while (!waiting.empty()) {
      for (const auto &wstate: waiting.front()->next[0]) {
        auto state = wstate.second.lock();
        if (state && closure.find(state) == closure.end()) {
          closure.insert(state);
          waiting.push_back(state);
        }
      }
      waiting.pop_front();
    }
  }

  struct ZoneAutomaton : public Automaton<ZAState> {
    using State = ZAState;

    //! @brief remove states unreachable to any accepting states
    void removeDeadStates() {
      // Find states unreachable to any accepting states
      using Config = std::pair<std::shared_ptr<ZAState>,
              std::unordered_set<std::shared_ptr<ZAState>>>;
      std::stack<Config> States;
      for (const auto &state: initialStates) {
        States.push(Config(state, {}));
      }
      std::unordered_set<std::shared_ptr<ZAState>> reachable;
      while (!States.empty()) {
        Config conf = States.top();
        States.pop();
        if (conf.first->isMatch) {
          reachable.insert(conf.first);
          reachable.insert(conf.second.begin(), conf.second.end());
        }
        for (const auto &edges: conf.first->next) {
          for (const auto &edge: edges) {
            if (conf.second.find(conf.first) != conf.second.end()) {
              // We are in a loop
              continue;
            }
            if (reachable.find(edge.second.lock()) != reachable.end()) {
              // The next state is reachable
              reachable.insert(conf.first);
              reachable.insert(conf.second.begin(), conf.second.end());
            } else {
              // The next state may be unreachable
              auto parents = conf.second;
              parents.insert(conf.first);
              States.push(Config(edge.second.lock(), parents));
            }
          }
        }
      }

      // Remove unreachable states
      for (auto it = states.begin(); it != states.end();) {
        if (reachable.find(*it) == reachable.end()) {
          it = states.erase(it);
        } else {
          it++;
        }
      }
      for (auto it = initialStates.begin(); it != initialStates.end();) {
        if (reachable.find(*it) == reachable.end()) {
          it = initialStates.erase(it);
        } else {
          it++;
        }
      }
    }

    /*!
      @brief Propagate accepting states from the original timed automaton
      @note taInitSates must be sorted
    */
    void updateInitAccepting(const std::vector<std::shared_ptr<TAState>> &taInitialStates) {
      // update initial states
      initialStates.clear();
      for (std::shared_ptr<ZAState> s: states) {
        if (std::find_if(taInitialStates.begin(), taInitialStates.end(),
                         [&](const std::shared_ptr<TAState> &taS) {
                           return taS.get() == s->taState;
                         }) != taInitialStates.end()) {
          initialStates.push_back(s);
        }
      }

      // update accepting states
      for (auto &state: states) {
        state->isMatch = state->taState->isMatch;
      }
    }

    //! @brief emptiness check of the language
    [[nodiscard]] bool empty() const {
      std::vector<std::shared_ptr<ZAState>> currentStates = initialStates;
      std::unordered_set<std::shared_ptr<ZAState>> visited = {
              initialStates.begin(), initialStates.end()};
      while (!currentStates.empty()) {
        std::vector<std::shared_ptr<ZAState>> nextStates;
        for (const auto &state: currentStates) {
          if (state->isMatch) {
            return false;
          }
          for (const auto &edges: state->next) {
            for (const auto &edge: edges) {
              auto target = edge.second.lock();
              if (target && visited.find(target) == visited.end()) {
                // We have not visited the state
                nextStates.push_back(target);
                visited.insert(target);
              }
            }
          }
        }
        currentStates = std::move(nextStates);
      }
      return true;
    }

    class SymbolicRun {
    private:
      std::vector<std::shared_ptr<ZAState>> states;
      std::vector<learnta::TATransition> edges;
      std::string word;
    public:
      explicit SymbolicRun(const std::shared_ptr<ZAState> &initialState) : states({initialState}), edges() {}

      void push_back(const learnta::TATransition& transition, char action, const std::shared_ptr<ZAState>& state) {
        this->states.push_back(state);
        word.push_back(action);
        this->edges.push_back(transition);
      }

      [[nodiscard]] std::shared_ptr<ZAState> stateAt(int i) const {
        return this->states.at(i);
      }

      [[nodiscard]] learnta::TATransition edgeAt(int i) const {
        return this->edges.at(i);
      }

      [[nodiscard]] std::shared_ptr<ZAState> back() const {
        return this->states.back();
      }

      /*!
       * @breif Reconstruct a timed word from a symbolic run
       *
       * We use the reconstruction algorithm in [Andre+, NFM'22]
       * We note that our zone construction is state --time_elapse--> intermediate --discrete_jump--> next_state.
       */
      [[nodiscard]] TimedWord reconstructWord() const {
        // Sample the last concrete state;
        auto postZoneState = this->back();
        auto postZone = postZoneState->zone;
        auto postValuation = postZone.sample();

        const auto N = this->edges.size();
        std::list<double> durations;

        for (int i = N - 1; i >= 0; --i) {
          auto preZoneState = this->stateAt(i);
          auto preZone = preZoneState->zone;

          // Construct the zone just before jump
          auto zoneBeforeJump = Zone{postValuation, postZone.M};
          const auto transition = this->edgeAt(i);
          for (const auto reset: transition.resetVars) {
            zoneBeforeJump.unconstrain(reset);
          }
          for (const auto guard: transition.guard) {
            zoneBeforeJump.tighten(guard);
          }
          {
            auto tmpPreZone = preZone;
            tmpPreZone.elapse();
            zoneBeforeJump &= tmpPreZone;
          }

          // Sample the valuation just before discrete jump
          const auto valuationBeforeJump = zoneBeforeJump.sample();
          auto backwardPreZone = Zone{valuationBeforeJump, postZone.M};
          backwardPreZone.reverseElapse();
          const auto preValuation = (preZone && backwardPreZone).sample();
          durations.push_front(valuationBeforeJump.at(0) - preValuation.at(0));

          // Update the variables
          postZone = std::move(preZone);
          postValuation = preValuation;
        }
        // Return the timed word
        std::vector<double> durationsVector;
        durationsVector.resize(durations.size());
        std::move(durations.begin(), durations.end(), durationsVector.begin());

        return {word, durationsVector};
      }
    };

    /*!
     * @brief Sample a timed word in this zone automaton
     *
     * We use the reconstruction algorithm in [Andre+, NFM'22]
     */
    [[nodiscard]] std::optional<TimedWord> sample() const {
      std::vector<SymbolicRun> currentStates;
      currentStates.resize(initialStates.size());
      std::transform(initialStates.begin(), initialStates.end(), currentStates.begin(),
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
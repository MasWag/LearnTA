/**
 * @author Masaki Waga
 * @date 2022/12/21.
 */

#pragma once

#include <vector>

#include "common_types.hh"
#include "timed_automaton.hh"

/**
 * @brief Fixture of an example OTA
 */
struct OTAExampleFixture {
  const std::vector<learnta::Alphabet> alphabet = {'a', 'b'};
  learnta::TimedAutomaton targetAutomaton, complementTargetAutomaton;

  OTAExampleFixture() {
    // Generate the target DTA
    /*
     * loc0 [init=false, match=true]
     * loc1 [init=true, match=false]
     */
    targetAutomaton.states.resize(2);
    targetAutomaton.states.at(0) = std::make_shared<learnta::TAState>(true);
    targetAutomaton.states.at(1) = std::make_shared<learnta::TAState>(false);
    targetAutomaton.initialStates.push_back(targetAutomaton.states.at(1));

    // Transitions
    // loc0->loc0 [label="b", guard="{x0 >= 2, x0 < 4}", reset="{x0 := 0}"]
    targetAutomaton.states.at(0)->next['b'].emplace_back();
    targetAutomaton.states.at(0)->next['b'].back().target = targetAutomaton.states.at(0).get();
    targetAutomaton.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) >= 2,
                                                            learnta::ConstraintMaker(0) < 4};
    targetAutomaton.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc1->loc1 [label="b", reset="{x0 := 0}"]
    targetAutomaton.states.at(1)->next['b'].emplace_back();
    targetAutomaton.states.at(1)->next['b'].back().target = targetAutomaton.states.at(1).get();
    targetAutomaton.states.at(1)->next['b'].back().resetVars.emplace_back(0, 0.0);
    // loc1->loc0 [label="a", guard="{x0 > 1, x0 < 3}"]
    targetAutomaton.states.at(1)->next['a'].emplace_back();
    targetAutomaton.states.at(1)->next['a'].back().target = targetAutomaton.states.at(0).get();
    targetAutomaton.states.at(1)->next['a'].back().guard = {learnta::ConstraintMaker(0) > 1,
                                                            learnta::ConstraintMaker(0) < 3};

    targetAutomaton.maxConstraints = {4};

    // Generate the complement of the target DTA
    complementTargetAutomaton = targetAutomaton.complement(this->alphabet);
  }
};

struct OTAExampleHypothesis20221221Fixture {
  learnta::TimedAutomaton hypothesis;

  OTAExampleHypothesis20221221Fixture() {
    // Generate the target DTA
    /*
     *  loc0 [init=true, match=false]
     *  loc1 [init=false, match=true]
     *  loc2 [init=false, match=true]
     *  loc3 [init=false, match=true]
     *  loc4 [init=false, match=true]
     *  loc5 [init=false, match=true]
     */
    hypothesis.states.resize(6);
    hypothesis.states.at(0) = std::make_shared<learnta::TAState>(true);
    for (int i = 1; i < 6; ++i) {
      hypothesis.states.at(i) = std::make_shared<learnta::TAState>(false);
    }
    hypothesis.initialStates.push_back(hypothesis.states.at(0));

    // Transitions
    // loc0->loc0 [label="b", guard="{x0 <= 0}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) <= 0};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc0->loc0 [label="b", guard="{x0 > 0, x0 < 1}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) > 0,
                                                       learnta::ConstraintMaker(0) < 1};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc0->loc0 [label="b", guard="{x0 >= 1, x0 <= 1}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) >= 1,
                                                       learnta::ConstraintMaker(0) <= 1};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc0->loc0 [label="b", guard="{x0 > 1, x0 < 2}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) > 1,
                                                       learnta::ConstraintMaker(0) < 2};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc0->loc0 [label="b", guard="{x0 >= 2, x0 <= 2}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) >= 2,
                                                       learnta::ConstraintMaker(0) <= 2};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc0->loc0 [label="b", guard="{x0 > 2, x0 < 3}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) > 2,
                                                       learnta::ConstraintMaker(0) < 3};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc0->loc0 [label="b", guard="{x0 >= 3, x0 <= 3}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) >= 3,
                                                       learnta::ConstraintMaker(0) <= 3};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    // loc0->loc0 [label="b", guard="{x0 > 3}", reset="{x0 := 0}"]
    hypothesis.states.at(0)->next['b'].emplace_back();
    hypothesis.states.at(0)->next['b'].back().target = hypothesis.states.at(0).get();
    hypothesis.states.at(0)->next['b'].back().guard = {learnta::ConstraintMaker(0) > 3};
    hypothesis.states.at(0)->next['b'].back().resetVars.emplace_back(0, 0.0);

    //        loc0->loc3 [label="a", guard="{x0 > 2, x0 < 3}", reset="{x1 := 0}"]
    //        loc0->loc2 [label="a", guard="{x0 >= 2, x0 <= 2}", reset="{x1 := 0}"]
    //        loc0->loc1 [label="a", guard="{x0 > 1, x0 < 2}", reset="{x1 := 0}"]
    //        loc1->loc3 [label="ε", guard="{x0 > 2, x1 > 0}", reset="{x1 := 0}"]
    //        loc1->loc4 [label="b", guard="{x0 >= 2, x0 <= 2, x1 > 0}", reset="{x2 := 0}"]
    //        loc2->loc5 [label="b", guard="{x0 >= 2, x0 <= 2, x1 <= 0}", reset="{x2 := 0}"]
    //        loc2->loc4 [label="b", guard="{x0 > 2, x0 < 3, x1 > 0, x1 < 1}", reset="{x0 := 2, x1 := 0.5, x2 := 0}"]
    //        loc2->loc4 [label="b", guard="{x0 >= 3, x0 <= 3, x1 >= 1, x1 <= 1}", reset="{x0 := 2, x1 := 0.5, x2 := 0}"]
    //        loc2->loc4 [label="b", guard="{x0 > 3, x0 < 4, x1 > 1, x1 < 2}", reset="{x0 := 2, x1 := 0.5, x2 := 0}"]
    //        loc3->loc4 [label="b", guard="{x0 > 2, x0 < 3, x1 <= 0}", reset="{x0 := 2, x1 := 0.5, x2 := 0}"]
    //        loc3->loc2 [label="b", guard="{x0 > 2, x1 > 0, x1 < 1}", reset="{x0 := 4, x1 := 2}"]
    //        loc3->loc2 [label="b", guard="{x0 > 3, x0 < 4, x1 >= 1, x1 <= 1}", reset="{x0 := 4, x1 := 2}"]
    //        loc4->loc5 [label="ε", guard="{x0 > 2, x1 > 0, x2 > 0}", reset="{x1 := 0.5, x2 := 0.5}"]
    //        loc5->loc1 [label="ε", guard="{x0 > 3, x1 > 1}", reset="{x0 := x1, x1 := 0.25}"]

    hypothesis.maxConstraints = {4};
  }
};
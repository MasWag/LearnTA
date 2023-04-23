How to use LearnTA for a custom system
======================================

This document briefly shows how to use LearnTA beyond our paper. Please also see files under [`examples/`](https://github.com/MasWag/LearnTA/tree/master/examples), for example [`learn_simple_dta.cc`](https://github.com/MasWag/LearnTA/blob/master/examples/learn_simple_dta.cc) and [`experiment_runner.hh`](https://github.com/MasWag/LearnTA/blob/master/examples/experiment_runner.hh), for concrete examples.

To learn a DTA with LearnTA, you need to do the following.

- Construct learnta::SUL wrapping the system under learning.
    - Currently, we only provide learnta::TimedAutomatonRunner, which is a wrapper of a DTA.
- Construct learnta::SULMembershipOracle from the SUL constructed above.
- Construct learnta::EquivalenceOracle
    - learnta::ComplementTimedAutomataEquivalenceOracle is the equivalence oracle based on a zone-based analysis.
- Construct learnta::Learner from a vector representing the alphabet, SymbolicMembershipPracle, and EquivalenceOracle.
- Run learnta::Learner::run


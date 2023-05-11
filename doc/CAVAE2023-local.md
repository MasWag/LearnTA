Artifact of "Active Learning of Deterministic Timed Automata with Myhill-Nerode Style Characterization"
=======================================================================================================

This is a document to reproduce the experimental results of our paper "Active Learning of Deterministic Timed Automata with Myhill-Nerode Style Characterization" without virtual environment.
LearnTA is a prototype tool implementing the proposed algorithm.
DOTALearningSMT (called OneSMT in the paper) is used in the experiment as a baseline method.

In what follows, we assume that the current directory is the root of LearnTA.

Requirements
------------

* C++ compiler supporting C++17 and the corresponding libraries.
* Boost (>= 1.59)
* Eigen
* CMake
* Python 3 and venv (for DOTALearningSMT)

Installation of LearnTA
-----------------------

1. Download the Git repository by `git clone --recursive https://github.com/MasWag/LearnTA.git`.
    - If you have already cloned the repository, please run `git submodule update --init --recursive`.
2. Build the examples by the following commands
    - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -- learn_simple_dta learn_ota_json learn_unbalanced_loop learn_fddi unit_test`
    - The resulting examples are build under `build/examples/`.
    - The unit_test is build at `build/unit_test`.

Installation of DOTALearningSMT
-------------------------------

1. Download the Git repository by `git clone --recursive https://github.com/MasWag/LearnTA.git`.
    - If you have already cloned the repository, please run `git submodule update --init --recursive`.
2. Apply patch to `DOTALearningSMT` by the following command:
    - `patch -d ./examples/DOTALearningSMT/ < ./examples/patch`.
3. Set up venv by: `python3 -m venv ./examples/DOTALearningSMT/.venv`.
4. Install required packages by: `. ./examples/DOTALearningSMT/.venv/bin/activate && pip install -r ./examples/requirements.txt`.

How to replicate the experiment results
---------------------------------------

1. Learn DTAs with LearnTA by running one of the following scripts under `./utils/`.
   - `run_LearnTA-all.sh` for all the benchmarks
       - This takes about 12 hours in total.
       - This works on a virtual machine with at least 64 GiB of RAM.
   - `run_LearnTA-huge.sh` for benchmarks other than `4_4_20`
       - This takes about 3,4 hours in total.
       - This works on a virtual machine with at least 64 GiB of RAM.
   - `run_LearnTA-large.sh` for benchmarks other than `4_4_20` and `FDDI`
       - This takes about 40 minutes.
       - This works on a virtual machine with at least 32 GiB of RAM.
   - `run_LearnTA-middle.sh` for benchmarks other than `4_4_20`
       - This also takes about 40 minutes.
       - This works on a virtual machine with at least 16 GiB of RAM.
   - `run_LearnTA-small.sh` for benchmarks other than `4_4_20`, `Unbalanced`, and `FDDI`
       - This takes about 15 minutes in total.
       - This works on a virtual machine with at least 8192 MiB of RAM.
2. Learn OCDTAs with DOTALearnSMT by running `./utils/run_DOTALearnSMT.sh`
   - This takes about five minutes in total.
3. Generate a table showing the experiment result by running `./utils/print_table.sh` on the Desktop and executing it
   - `./logs/table.html` will be generated on the Desktop
4. Open `table.html` by a Web browser

On reusability
--------------

We believe that our artifact is reusable because of the following reasons.

- The source code of LearnTA is distributed under GPLv3 at https://github.com/MasWag/LearnTA . The artifact is distributed under Apache 2.0 license.
- LearnTA depends on the following software and libraries. This information is also written in https://github.com/MasWag/LearnTA and https://maswag.github.io/LearnTA/doxygen/index.html .
    - C++ compiler supporting C++17 and the corresponding libraries.
    - Boost (>= 1.59)
    - Eigen
    - CMake
- The next section shows how to use LearnTA for benchmarks beyond the paper.
- The artifact is open source.
- The README of LearnTA shows how to build and run the examples in environments other than the VM.

How to use LearnTA for a custom benchmark
-----------------------------------------

This section briefly shows how to use LearnTA beyond our paper. Please also see files under [`examples/`](https://github.com/MasWag/LearnTA/tree/master/examples), for example [`learn_simple_dta.cc`](https://github.com/MasWag/LearnTA/blob/master/examples/learn_simple_dta.cc) and [`experiment_runner.hh`](https://github.com/MasWag/LearnTA/blob/master/examples/experiment_runner.hh), for concrete examples. This document is also available at https://maswag.github.io/LearnTA/doxygen/md_doc_usage.html .

To learn a DTA with LearnTA, you need to do the following.

- Construct learnta::SUL wrapping the system under learning.
    - Currently, we only provide learnta::TimedAutomatonRunner, which is a wrapper of a DTA.
- Construct learnta::SULMembershipOracle from the SUL constructed above.
- Construct learnta::EquivalenceOracle
    - learnta::ComplementTimedAutomataEquivalenceOracle is the equivalence oracle based on a zone-based analysis.
- Construct learnta::Learner from a vector representing the alphabet, SymbolicMembershipPracle, and EquivalenceOracle.
- Run learnta::Learner::run

API document
------------

As a library, we believe that it is crucial to provide a document of the classes, functions, and so on. The API document generated by doxygen is available from https://maswag.github.io/LearnTA/doxygen/index.html .

Unit test
---------

We use unit tests to ensure the quality of the code of LearnTA. The codes for the unit test are under `./LearnTA/test/`. You can execute the unit test by `cd ./build && make unit_test && ./unit_test`.

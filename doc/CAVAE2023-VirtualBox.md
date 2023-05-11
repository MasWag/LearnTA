Artifact of "Active Learning of Deterministic Timed Automata with Myhill-Nerode Style Characterization"
=======================================================================================================

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.7875383.svg)](https://doi.org/10.5281/zenodo.7875383)

This is a document of the artifact of our paper "Active Learning of Deterministic Timed Automata with Myhill-Nerode Style Characterization". 
LearnTA is a prototype tool implementing the proposed algorithm.
DOTALearningSMT (called OneSMT in the paper) is used in the experiment as a baseline method.

The artifact is available from https://doi.org/10.5281/zenodo.7875383 .

Content of the artifact
-----------------------

The artifact (`CAV2023AE.ova`) is an image of a virtual machine with Ubuntu 22.04. One can run it, for example, using VirtualBox (https://www.virtualbox.org/ ).

The artifact consists of the following contents.

- The source code and the required packages for `LearnTA`
    - The source code: `/home/vagrant/Desktop/LearnTA`
- The source code and the required packages for `DOTALearningSMT`
    - The original source code: `/home/vagrant/Desktop/DOTALearningSMT`
    - An entry point to execute it: `/home/vagrant/Desktop/runDOTALearningSMT.py`
    - A patch we made: `/home/vagrant/patch`
- Scripts to automate the execution of the experiments
    - LearnTA: `/home/vagrant/Desktop/run_LearnTA-all.sh`
        - This takes about 12 hours in total.
        - This works on a virtual machine with at least 64 GiB of RAM.
    - LearnTA (for benchmarks other than `4_4_20`): `/home/vagrant/Desktop/run_LearnTA-huge.sh`
        - This takes about 3,4 hours in total.
        - This works on a virtual machine with at least 64 GiB of RAM.
    - LearnTA (for benchmarks other than `4_4_20` and `FDDI`): `/home/vagrant/Desktop/run_LearnTA-large.sh`
        - This takes about 40 minutes.
        - This works on a virtual machine with at least 32 GiB of RAM.
    - LearnTA (for benchmarks other than `4_4_20`, `Unbalanced:5`, and `FDDI`): `/home/vagrant/Desktop/run_LearnTA-middle.sh`
        - This also takes about 40 minutes.
        - This works on a virtual machine with at least 16 GiB of RAM.
    - LearnTA (for benchmarks other than `4_4_20`, `Unbalanced`, and `FDDI`): `/home/vagrant/Desktop/run_LearnTA-small.sh`
        - This takes about 15 minutes.
        - This works on a virtual machine with at least 8 GiB of RAM.
    - DOTALearningSMT: `/home/vagrant/Desktop/run_DOTALearningSMT.sh`
    - The logs are saved under the `/home/vagrant/Desktop/logs`
- Script to make a table of the experiment results
    - The script to run: `/home/vagrant/Desktop/print_table.sh`, which depends on the following codes
        - `/home/vagrant/to_json.awk`
        - `/home/vagrant/to_json-octa.awk`
        - `/home/vagrant/print_table.py`
- A toolkit to visualize the DTA learned by LearnTA
    - From GUI: `/home/vagrant/Desktop/render-launch.sh`
    - From command line: `/home/vagrant/Desktop/render.sh`
    - Internal library: `/home/vagrant/Desktop/LearnTA-dot`
- The submitted paper
    - `/home/vagrant/Desktop/cav23-paper127.pdf`

How to set up the artifact
--------------------------

1. Install VirtualBox ( https://www.virtualbox.org/ )
2. Create a virtual machine using `CAV2023AE.ova`. 
    - We used the following configuration.
        - Number of processors: 2
        - RAM: 64GiB (65536 MB)
    - Some benchmarks work on a virtual machine with smaller RAM, e.g., 16GiB.
3. Start the virtual machine
4. Log in to it with the following information
   - username: vagrant
   - password: cav

Environment we used to test the artifact
----------------------------------------

- CPU: Intel(R) Core(TM) i9-10980XE CPU @ 3.00GHz with 36 cores
- RAM: 125GiB
- OS: Ubuntu 20.04

How to replicate the experiment results
---------------------------------------

1. Learn DTAs with LearnTA by clicking one of the following files on the Desktop and executing it
   - `run_LearnTA-all.sh` for all the benchmarks
       - This takes about 12 hours in total.
       - This works on a virtual machine with at least 64 GiB of RAM.
   - `run_LearnTA-huge.sh` for benchmarks other than `4_4_20`
       - This takes about 3,4 hours in total.
       - This works on a virtual machine with at least 64 GiB of RAM.
   - `run_LearnTA-large.sh` for benchmarks other than `4_4_20` and `FDDI`
       - This takes about 40 minutes.
       - This works on a virtual machine with at least 32 GiB of RAM.
   - `run_LearnTA-middle.sh` for benchmarks other than `4_4_20`, `FDDI`, and `Unbalanced` with clock size 5.
       - This also takes about 40 minutes.
       - This works on a virtual machine with at least 16 GiB of RAM.
   - `run_LearnTA-small.sh` for benchmarks other than `4_4_20`, `Unbalanced`, and `FDDI`
       - This takes about 15 minutes in total.
       - This works on a virtual machine with at least 8192 MiB of RAM.
2. Learn OCDTAs with DOTALearnSMT by clicking `run_DOTALearningSMT.sh` on the Desktop and executing it
   - This takes about five minutes in total.
3. Generate a table showing the experiment result by clicking `/home/vagrant/Desktop/print_table.sh` on the Desktop and executing it
   - `table.html` will be generated on the Desktop
4. Open `table.html` by clicking it (Firefox should open)

### Additional step

If you want to render the learned DTA, click `render-launch.sh` on the Desktop and execute it. A dialog to choose the log files will open. The generated image files are saved under `/home/vagrant/Desktop/rendered`.

### Running from command line

All the scripts above can also be executed from a terminal emulator, for example, lxterminal. Moreover, we have the following scripts for command line interface.

- `/home/vagrant/Desktop/run_LearnTA.sh`: Learn the DTAs in the given benchmarks with LearnTA
    - Example: `cd /home/vagrant/Desktop/ && ./run_LearnTA.sh 3_2_10 AKM Train` to run `3_2_10`, `AKM`, and `Train`
- `/home/vagrant/Desktop/render.sh`: Render DTAs with the given log files of LearnTA
    - Example: `cd /home/vagrant/Desktop/ && ./render.sh ./logs/LearnTA/AKM Train` to render a DTA for AKM

Source code of LearnTA
----------------------

- Location: /home/vagrant/Desktop/LearnTA-CAV2023-submission

On availability
---------------

Our artifact, including the VM image, is uploaded to Zenodo and publicly available () with DOI. The source code is also available from GitHub ( https://github.com/MasWag/LearnTA ).

On reusability
--------------

We believe that our artifact is reusable because of the following reasons.

- The source code of LearnTA is distributed under GPLv3 at https://github.com/MasWag/LearnTA . The artifact is distributed under Apache 2.0 license.
- LearnTA depends on the following software and libraries. This information is also written in https://github.com/MasWag/LearnTA and https://maswag.github.io/LearnTA/doxygen/index.html .
    - C++ compiler supporting C++17 and the corresponding libraries.
    - Boost (>= 1.59)
    - Eigen
    - TBB
    - CMake
    - For document generation, doxygen and Graphviz are also required, but they are optional
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

We use unit tests to ensure the quality of the code of LearnTA. The codes for the unit test are under /home/vagrant/Desktop/LearnTA-CAV2023-submission/test/. You can execute the unit test by `cd /home/vagrant/Desktop/LearnTA-CAV2023-submission/build && ninja unit_test && ./unit_test`.

License
-------

The VM image `CAV2023AE.ova` is distributed under GPLv3.

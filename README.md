LearnTA
=======

[![Boost.Test](https://github.com/MasWag/LearnTA/actions/workflows/boosttest.yml/badge.svg?branch=master)](https://github.com/MasWag/LearnTA/actions/workflows/boosttest.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](./LICENSE)
[![Doxygen](https://img.shields.io/badge/docs-Doxygen-deepgreen.svg)](https://maswag.github.io/LearnTA/doxygen/index.html)

This is the source code repository for LearnTA --- A C++ library for active learning of deterministic timed automata.

Installation
------------

LearnTA is tested on Ubuntu (20.04 and 22.04) and macOS (Catalina 10.15, Big Sur 11, and Monterey 12).

### Requirements

* C++ compiler supporting C++17 and the corresponding libraries.
* Boost (>= 1.59)
* Eigen
* TBB
* CMake

To generate the document, Doxygen and graphviz are also required.

### Instructions

```sh
mkdir build
cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make
```

By default, it does not build any executable examples. To build them, please run, for example, the following command.

```sh
make learn_simple_dta learn_ota_json learn_unbalanced_loop learn_fddi unit_test
```

How to run examples
-------------------

The examples are in [`./examples`](./examples). See [`./doc/CAVAE2023-local.md`](./doc/CAVAE2023-local.md) how to reproduce the experimental results of our CAV paper.

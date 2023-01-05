LearnTA
=======

[![Boost.Test](https://github.com/MasWag/LearnTA/actions/workflows/boosttest.yml/badge.svg?branch=master)](https://github.com/MasWag/LearnTA/actions/workflows/boosttest.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](./LICENSE)
[![Doxygen](https://img.shields.io/badge/docs-Doxygen-deepgreen.svg)](https://maswag.github.io/LearnTA/doxygen/index.html)

This is the source code repository for LearnTA --- A C++ library for active learning of deterministic timed automata.

Installation
------------

LearnTA is tested on Ubuntu (20.04 and 22.04) and macOS 10.15 Catalina.

### Requirements

* C++ compiler supporting C++17 and the corresponding libraries.
* Boost (>= 1.59)
* Eigen
* CMake

### Instructions

```sh
mkdir build
cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make
```

How to run examples
-------------------

The examples are in [`./examples`](./examples).

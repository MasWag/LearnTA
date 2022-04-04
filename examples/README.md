Examples
=========

This directory contains example usage of LearnTA.

Files
------

Each `.cc` file contains one example.

- learn_simple_dta.cc
  - The running example in our paper.  
- learn_light.cc
  - The `light` benchmark taken from [[APT'20]](https://doi.org/10.1007/978-3-030-55754-6_1).
- learn_CAS.cc
   - The `CAS` benchmark taken from [[APT'20]](https://doi.org/10.1007/978-3-030-55754-6_1).
- learn_PC.cc
  - The `PC` benchmark taken from [[APT'20]](https://doi.org/10.1007/978-3-030-55754-6_1).

Usage
-----

```sh
mkdir -p ../build
cd ../build && cmake -DCMAKE_BUILD_TYPE=Release .. && make learn_<NAME>.cc
./examples/learn_<NAME>
```

References
-----------

- [APT'20]: Aichernig, Bernhard K., Andrea Pferscher, and Martin Tappler. "From passive to active: learning timed automata efficiently." NASA Formal Methods Symposium. Springer, Cham, 2020.

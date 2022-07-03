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
cd ../build && cmake -DCMAKE_BUILD_TYPE=Release .. && make learn_<NAME>
./examples/learn_<NAME>
```

How to reproduce the experiment result
--------------------------------------

Here, we show how to reproduce the experiment result in our paper, i.e., Table 1 of [Waga'22].

First, build the binaries following the above instruction.

```sh
mkdir -p ../build
cd ../build && cmake -DCMAKE_BUILD_TYPE=Release .. && make learn_simple_dta learn_light learn_CAS learn_PC
```

Then, run each benchmark 30 times. You can manually run each benchmark if you want, but the following shell script automatically runs all the experiments.

```sh
for benchmark in simple_dta light CAS PC; do
    for i in $(seq 30); do
        ./examples/learn_${benchmark} > ../examples/logs/${benchmark}-$i.log ;
    done
done
```

You can summarize the experiment result by standard command line tools. The following shows an example.

```sh
for benchmark in simple_dta light CAS PC; do
    printf "Here are the summary of $benchmark\n"
    # The algorithm is deterministic, and these values do not change.
    cat ../examples/logs/${benchmark}-*.log | grep '|P|' | uniq
    cat ../examples/logs/${benchmark}-*.log | grep '|S|' | uniq
    cat ../examples/logs/${benchmark}-*.log | grep 'membership' | uniq
    cat ../examples/logs/${benchmark}-*.log | grep 'equivalence'  | uniq
    # We take the average for the execution time.
    awk '/Execution Time/{sum += $3;count +=1} END {print "Mean exec. time [ms]: " sum/count}' ../examples/logs/${benchmark}-*.log
done
```

References
-----------

- [APT'20]: Aichernig, Bernhard K., Andrea Pferscher, and Martin Tappler. "From passive to active: learning timed automata efficiently." NASA Formal Methods Symposium. Springer, Cham, 2020.
- [Waga'22]: Waga, Masaki. "L*-Style Active Learning of Deterministic Timed Automata."

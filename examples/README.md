Examples
=========

This directory contains example usage of LearnTA.

Files and directories
---------------------

The following files and directories are relevant to reproduce the result in our paper [Waga'23]. Please execute `git submodule update --init` if `OTALearning` and `DOTALearningSMT` are empty.

- learn_simple_dta.cc
    - The running example in our paper.
- learn_ota_json.cc
    - Learn a DTA from a JSON file representing the target DOTA in the format used in `OTALearning` and `DOTALearningSMT`.
- learn_unbalanced_loop.cc
    - The `Unbalanced` benchmark
- learn_fddi.cc
    - The `FDDI` benchmark
    - Note: it takes a "scaling parameter" to change the complexity of the benchmark. The default parameter 20 is extremely complex. We used the parameter 1 in the paper.
- `OTALearning`
    - Directory containing the artifact of [ACZZZ'20]. Some of the benchmarks are taken from it.
- `DOTALearningSMT`
    - Directory containing the artifact of [XAZ'22]. This is the baseline implementation used in our experiments. Moreover, some of the benchmarks are taken from it.

The following files are not used in the experiments in [Waga'23] but demonstrating the usage of LearnTA.

- learn_light.cc
  - The `light` benchmark taken from [[APT'20]](https://doi.org/10.1007/978-3-030-55754-6_1).
- learn_CAS.cc
   - The `CAS` benchmark taken from [[APT'20]](https://doi.org/10.1007/978-3-030-55754-6_1).
- learn_PC.cc
  - The `PC` benchmark taken from [[APT'20]](https://doi.org/10.1007/978-3-030-55754-6_1).

Building the examples
---------------------

These examples can also be built in the same way as LearnTA.

```sh
cmake -S .. -B ../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../build -- learn_<NAME>
../build/examples/learn_<NAME>
```

Usage
-----

Most of the examples can be run by executing the built file. For example, `learn_simple_dta` can be executed as follows.

```sh
# The first two lines are unnecessary if learn_simple_dta is already built.
cmake -S .. -B ../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../build -- learn_simple_dta
../build/examples/learn_simple_dta
```

`learn_ota_json` takes a path to the JSON file to use. For example, the DOTA `3_2_10-1.json` can be learned as follows.

```sh
# The first two lines are unnecessary if learn_ota_json is already built.
cmake -S .. -B ../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../build -- learn_ota_json
../build/examples/learn_ota_json ./OTALearning/experiments/3_2_10/3_2_10-1.json
```

When you use the JSON files under `DOTALearningSMT`, you first have to rename the alphabet. That can be done by as follows using jq.

```bash
for json in ./DOTALearningSMT/examples/DOTA/OTAs/*.json; do 
    jq '.sigma as $sigma | (.tran |= with_entries(.value[1] = (.value[1] as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))]))) | (.sigma |= map(. as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))])) | .' "$json" > ${json##*/}
done
```

`learn_unbalanced_loop` takes the number of locations and the clock variables of the DTA, and the parameter to scale the maximum constraints. For example, the DTA `Unbalanced:3` in [Waga'23] can be learned as follows.

```sh
# The first two lines are unnecessary if learn_unbalanced_loop is already built.
cmake -S .. -B ../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../build -- learn_unbalanced_loop
../build/examples/learn_unbalanced_loop 5 3 1
```

`learn_fddi` takes the parameter to scale the maximum constraints. For example, the DTA `FDDI` in [Waga'23] can be learned as follows.

```sh
# The first two lines are unnecessary if learn_unbalanced_loop is already built.
cmake -S .. -B ../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../build -- learn_fddi
../build/examples/learn_fddi 1
```

How to reproduce the experiment results
---------------------------------------

Here, we show how to reproduce some of the experiment results in our paper. Please see https://github.com/MasWag/LearnTA-ae for a more elaborated instructions and materials.

First, build the executable files following the above instruction.

```sh
cmake -S .. -B ../build -DCMAKE_BUILD_TYPE=Release
cmake --build ../build -- learn_ota_json learn_unbalanced_loop learn_fddi
```

Then, run each benchmark. You can manually run each benchmark if you want, but the following shell scripts automatically run all the experiments.

```sh
## Run all the benchmarks from OTALearning
mkdir -p logs
## You may want to exclude 4_4_20 because it takes very long time.
for benchmark in 3_2_10 4_2_10 4_4_20 5_2_10 6_2_10; do
    for json in "./OTALearning/experiments/$benchmark/"*.json; do
        ../build/examples/learn_ota_json $json > logs/${json##*/}.log
    done
done
```

```sh
## Run all the benchmarks from DOTALearningSMT
mkdir -p logs
for json in ./DOTALearningSMT/examples/DOTA/OTAs/*.json; do 
    jq '.sigma as $sigma | (.tran |= with_entries(.value[1] = (.value[1] as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))]))) | (.sigma |= map(. as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))])) | .' "$json" > ${json##*/}
    ../build/examples/learn_ota_json ${json##*/} > logs/${json##*/}.log
done
```

```sh
## Run the Unbalanced benchmark
mkdir -p logs
for clock in $(seq 1 5); do 
    ../build/examples/learn_unbalanced_loop 5 $clock 1 > logs/unbalanced-$clock-$i.log
done
```

```sh
## Run the FDDI benchmark
mkdir -p logs
../build/examples/learn_fddi 1 > logs/fddi.log
```

<!--
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
-->

Expected time to execute each command
-------------------------------------

Except for `4_4_20`, `FDDI` and `Unbalanced`, each command would take at most around 5 to 10 minutes. They work on a computer with 8192 MiB of RAM.

For `Unbalanced`, it takes at most about 20-30 minutes. It takes significantly long time and large RAM for `Unbalanced` with 5 clocks, i.e., `../build/examples/learn_unbalanced_loop 5 5 1`. They work on a computer with 16 GiB of RAM for 1-4 clocks and with 32 GiB of RAM for 5 clocks.

For `FDDI`, it takes at most about 3-4 hours. It works on a computer with 64 GiB of RAM.

For `4_4_20`, it takes at most about 12 hours. It works on a computer with 64 GiB of RAM.

References
-----------

- [ACZZZ'20]: An, J., Chen, M., Zhan, B., Zhan, N., Zhang, M. (2020). Learning One-Clock Timed Automata. In: Biere, A., Parker, D. (eds) Tools and Algorithms for the Construction and Analysis of Systems. TACAS 2020.
- [XAZ'22]: Xu, R., An, J., Zhan, B. (2022). Active Learning of One-Clock Timed Automata Using Constraint Solving. In: Bouajjani, A., Holík, L., Wu, Z. (eds) Automated Technology for Verification and Analysis. ATVA 2022.
- [APT'20]: Aichernig, Bernhard K., Andrea Pferscher, and Martin Tappler. "From passive to active: learning timed automata efficiently." NASA Formal Methods Symposium. Springer, Cham, 2020.
- [Waga'23]: Waga, Masaki. "Active Learning of Deterministic Timed Automata with Myhill-Nerode Style Characterization." To appear in Proc. CAV 2023.

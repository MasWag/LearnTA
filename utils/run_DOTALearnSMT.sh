#!/bin/bash -u
#****h* batches/run_DOTALearningSMT
# NAME
#  run_DOTALearningSMT
# DESCRIPTION
#  Execute DOTALearningSMT
#
# USAGE
#  ./run_DOTALearnSMT.sh [benchmarks]
#
#******

ROOT=$(cd "$(dirname "$0")" && pwd)
readonly ALL_BENCHMARKS='3_2_10 4_2_10 4_4_20 5_2_10 6_2_10 AKM CAS Light PC TCP Train'
readonly TACAS_BENCHMARK_ROOT="${ROOT}/../examples/OTALearning/experiments"
readonly ATVA_BENCHMARK_ROOT="${ROOT}/../examples/DOTALearningSMT/examples/DOTA/OTAs"
readonly LOG_DIR="${ROOT}/../logs/DOTALearningSMT"
readonly TIMEOUT="timeout 3h"

# activate venv
. "${ROOT}/../examples//DOTALearningSMT/.venv/bin/activate"

if [ $# -eq 0 ]; then
    printf "Run DOTALearningSMT with all the benchmarks\n"
    benchmarks="$ALL_BENCHMARKS"
else
    printf "Run DOTALearningSMT with $@\n"
    benchmarks="$@"
fi    

mkdir -p "$LOG_DIR"
for benchmark in $benchmarks; do
    printf "Run $benchmark\n"
    if [[ $benchmark == *_* ]]; then
        for json in "${TACAS_BENCHMARK_ROOT}/${benchmark}/"*.json; do
            filename=${json##*/}
            $TIMEOUT python "${ROOT}/../examples/runDOTALearningSMT.py" $json | tee "$LOG_DIR/${filename/.json/}.log"
        done
    elif [ $(echo ${benchmark} | tr [a-z] [A-Z]) == LIGHT ]; then
        $TIMEOUT python "${ROOT}/../examples/runDOTALearningSMT.py" "$ATVA_BENCHMARK_ROOT/Light.json" | tee "$LOG_DIR/Light.log"
    elif [ $(echo ${benchmark} | tr [a-z] [A-Z]) == TRAIN ]; then
        $TIMEOUT python "${ROOT}/../examples/runDOTALearningSMT.py" "$ATVA_BENCHMARK_ROOT/Train.json" | tee "$LOG_DIR/Train.log"
    else
        $TIMEOUT python "${ROOT}/../examples/runDOTALearningSMT.py" "$ATVA_BENCHMARK_ROOT/$(echo ${benchmark} | tr [a-z] [A-Z]).json" | tee "$LOG_DIR/$(echo ${benchmark} | tr [a-z] [A-Z]).log"
    fi
done

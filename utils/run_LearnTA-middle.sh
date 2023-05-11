#!/bin/sh -ue

ROOT=$(cd "$(dirname "$0")" && pwd)
"${ROOT}/run_LearnTA.sh" 3_2_10 4_2_10 5_2_10 6_2_10 Unbalanced AKM CAS Light PC TCP Train

# Execute unbalanced 1-4
readonly EXECUTABLES_DIR="${ROOT}/../build"
readonly TACAS_BENCHMARK_ROOT="${ROOT}/../examples/OTALearning/experiments"
readonly ATVA_BENCHMARK_ROOT="${ROOT}/../examples/DOTALearningSMT/examples/DOTA/OTAs"
readonly LOG_DIR="${ROOT}/../logs/LearnTA"
readonly TIMEOUT="timeout 3h"

mkdir -p "$LOG_DIR"
for i in $(seq 1 4); do
    $TIMEOUT "${EXECUTABLES_DIR}/examples/learn_unbalanced_loop" 5 "$i" 1 | tee "$LOG_DIR/unbalanced-$i.log"
done

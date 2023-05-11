#!/bin/bash -u
#****h* batches/run_LearnTA
# NAME
#  run_LearnTA
# DESCRIPTION
#  Execute LearnTA
#
# USAGE
#  ./run_LearnTA.sh [benchmarks]
#
#******

ROOT=$(cd "$(dirname "$0")" && pwd)
readonly ALL_BENCHMARKS='3_2_10 4_2_10 4_4_20 5_2_10 6_2_10 Unbalanced AKM CAS Light PC TCP Train FDDI'
readonly EXECUTABLES_DIR="${ROOT}/../build"
readonly TACAS_BENCHMARK_ROOT="${ROOT}/../examples/OTALearning/experiments"
readonly ATVA_BENCHMARK_ROOT="${ROOT}/../examples/DOTALearningSMT/examples/DOTA/OTAs"
readonly LOG_DIR="${ROOT}/../logs/LearnTA"
readonly TIMEOUT="timeout 3h"

if [ $# -eq 0 ]; then
    printf "Run LearnTA with all the benchmarks\n"
    benchmarks="$ALL_BENCHMARKS"
else
    printf "Run LearnTA with $@\n"
    benchmarks="$@"
fi    

mkdir -p "$LOG_DIR"
for benchmark in $benchmarks; do
    printf "Run $benchmark\n"
    if [ ${benchmark@U} == UNBALANCED ]; then
        for i in $(seq 1 5); do
            $TIMEOUT "${EXECUTABLES_DIR}/examples/learn_unbalanced_loop" 5 "$i" 1 | tee "$LOG_DIR/unbalanced-$i.log"
        done
    elif [ ${benchmark@U} == FDDI ]; then
        $TIMEOUT "${EXECUTABLES_DIR}/examples/learn_fddi" 1 | tee "$LOG_DIR/fddi.log"
    elif [[ $benchmark == *_* ]]; then
        for json in "${TACAS_BENCHMARK_ROOT}/${benchmark}/"*.json; do
            filename=${json##*/}
            $TIMEOUT "${EXECUTABLES_DIR}/examples/learn_ota_json" $json | tee "$LOG_DIR/${filename/.json/}.log"
        done
    elif [ ${benchmark@U} == TRAIN ]; then
        jq '.sigma as $sigma | (.tran |= with_entries(.value[1] = (.value[1] as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))]))) | (.sigma |= map(. as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))])) | .' "$ATVA_BENCHMARK_ROOT/Train.json" > /tmp/Train.json
        $TIMEOUT "${EXECUTABLES_DIR}/examples/learn_ota_json" /tmp/Train.json | tee "$LOG_DIR/Train.log"
    elif [ ${benchmark@U} == LIGHT ]; then
        jq '.sigma as $sigma | (.tran |= with_entries(.value[1] = (.value[1] as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))]))) | (.sigma |= map(. as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))])) | .' "$ATVA_BENCHMARK_ROOT/Light.json" > /tmp/Light.json
        $TIMEOUT "${EXECUTABLES_DIR}/examples/learn_ota_json" /tmp/Light.json | tee "$LOG_DIR/Light.log"
    else
        jq '.sigma as $sigma | (.tran |= with_entries(.value[1] = (.value[1] as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))]))) | (.sigma |= map(. as $event | "abcdefghijklmnopqrstuvwxyz" | split("")[($sigma | index($event))])) | .' "$ATVA_BENCHMARK_ROOT/${benchmark@U}.json" > "/tmp/${benchmark@U}.json"
        $TIMEOUT "${EXECUTABLES_DIR}/examples/learn_ota_json" "/tmp/${benchmark@U}.json" | tee "$LOG_DIR/${benchmark@U}.log"
    fi
done

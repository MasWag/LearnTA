#!/bin/sh -ue

ROOT=$(cd "$(dirname "$0")" && pwd)
"$ROOT/run_LearnTA.sh" 3_2_10 4_2_10 5_2_10 6_2_10 AKM CAS Light PC TCP Train

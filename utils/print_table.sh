#!/bin/sh -ue

ROOT=$(cd "$(dirname "$0")" && pwd)
python3 -m venv "$ROOT/.venv"
. "$ROOT/.venv/bin/activate"
pip install pandas

# Make the JSON file
"$ROOT/to_json.awk" "$ROOT/../logs/LearnTA"/*.log > "$ROOT/../logs/LearnTA-results.json"
"$ROOT/to_json-octa.awk" "$ROOT/../logs/DOTALearningSMT"/*.log > "$ROOT/../logs/DOTALearningSMT-results.json"

# Make the table
python "$ROOT/print_table.py"

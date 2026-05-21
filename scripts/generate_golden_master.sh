#!/usr/bin/env bash
# Generates tests/golden_master_expected.txt from UnitConverter stdout.
# Usage (from repo root):
#   ./scripts/generate_golden_master.sh [build_dir]

set -euo pipefail

BUILD_DIR="${1:-build}"
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EXE="${REPO_ROOT}/${BUILD_DIR}/UnitConverter"
OUT="${REPO_ROOT}/tests/golden_master_expected.txt"
PROMPT_PREFIX="Insert value for converting"

if [[ ! -x "$EXE" ]]; then
  echo "UnitConverter not found: $EXE" >&2
  echo "Run: cmake -S . -B ${BUILD_DIR} -DUNIT_CONVERTER_RED_PHASE=OFF && cmake --build ${BUILD_DIR}" >&2
  exit 1
fi

strip_prompt_from_line() {
  local line="$1"
  if [[ "$line" == ERR-* ]]; then
    return 0
  fi
  if [[ "$line" == "${PROMPT_PREFIX}"* ]]; then
    local marker="): "
    local rest="${line#*${marker}}"
    if [[ "$rest" != "$line" && -n "$rest" ]]; then
      printf '%s\n' "$rest"
    fi
    return 0
  fi
  printf '%s\n' "$line"
}

capture_scenario() {
  local scenario="$1"
  local raw
  raw="$("$EXE" <<<"$scenario" 2>/dev/null || true)"
  while IFS= read -r line; do
    strip_prompt_from_line "$line"
  done <<<"$raw"
}

{
  for scenario in "meter:2.5" "feet:1.0" "yard:1.0" "meter:0.0"; do
    printf '[%s]\n' "$scenario"
    body="$(capture_scenario "$scenario")"
    if [[ -n "${body// }" ]]; then
      printf '%s\n' "$body"
    fi
    printf '%s\n' "---"
  done
} >"$OUT"

echo "Wrote $OUT"
echo "Stage for git: git add tests/golden_master_expected.txt"

#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-build}"
CONFIG="${CONFIG:-Debug}"
QT_PATH="${QT_PATH:-${CMAKE_PREFIX_PATH:-}}"
WITH_APP="${WITH_APP:-0}"

for candidate in \
  "$HOME/Qt/6.5.3/macos" \
  "$HOME/Qt/6.5.3/clang_64" \
  "/opt/homebrew/opt/qt@6"
do
  if [[ -z "$QT_PATH" && ( -d "$candidate/lib/cmake/Qt6" || -f "$candidate/lib/QtCore.framework/QtCore" ) ]]; then
    QT_PATH="$candidate"
  fi
done

if [[ -z "$QT_PATH" ]]; then
  echo "Qt 6 not found. Set QT_PATH." >&2
  exit 1
fi

cd "$ROOT_DIR"
export CMAKE_PREFIX_PATH="$QT_PATH"
export PATH="$QT_PATH/bin:$PATH"

echo "Sprint 1 test runner"
cmake -S . -B "$BUILD_DIR" -DCMAKE_PREFIX_PATH="$QT_PATH"

if [[ "$WITH_APP" == "1" ]]; then
  cmake --build "$BUILD_DIR" --config "$CONFIG"
else
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_dsp_processor
fi

ctest --test-dir "$BUILD_DIR" -C "$CONFIG" --output-on-failure -R dsp_processor
echo "Sprint 1 DSP tests passed."

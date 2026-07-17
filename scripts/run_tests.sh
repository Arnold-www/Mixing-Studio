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
  "/opt/homebrew/opt/qt@6" \
  "/opt/homebrew/opt/qtbase" \
  "/opt/homebrew"
do
  if [[ -z "$QT_PATH"
        && -d "$candidate/lib/cmake/Qt6"
        && -d "$candidate/lib/cmake/Qt6Qml"
        && -d "$candidate/lib/cmake/Qt6Multimedia" ]]; then
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

echo "Test runner"
cmake -S . -B "$BUILD_DIR" -DCMAKE_PREFIX_PATH="$QT_PATH"

if [[ "$WITH_APP" == "1" ]]; then
  cmake --build "$BUILD_DIR" --config "$CONFIG"
else
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target MixingStudio
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_dsp_processor
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_audio_engine
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_project_store
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_asset_library
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_common_types
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_commands
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_wav_export
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_wav_decoder
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_automation
  cmake --build "$BUILD_DIR" --config "$CONFIG" --target test_audio_pipeline_e2e
fi

ctest --test-dir "$BUILD_DIR" -C "$CONFIG" --output-on-failure
echo "All tests passed."

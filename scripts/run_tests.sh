#!/usr/bin/env bash
set -euo pipefail
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir" -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" -j
ctest --test-dir "$build_dir" --output-on-failure

#!/usr/bin/env bash
set -euo pipefail
build_dir=${BUILD_DIR:-build}
cmake -S . -B "$build_dir" -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" -j --target bench_spsc
"./$build_dir/bench_spsc"

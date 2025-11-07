#!/usr/bin/env bash
set -euo pipefail
build_dir=${BUILD_DIR:-build}

cmake -S . -B "$build_dir" -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build "$build_dir" -j

# Ajoute l'option -C Debug pour MSVC
ctest --test-dir "$build_dir" -C Debug --output-on-failure

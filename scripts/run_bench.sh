#!/usr/bin/env bash
set -euo pipefail

# Répertoire de build configurable (par défaut "build")
build_dir=${BUILD_DIR:-build}

# Configuration et compilation
cmake -S . -B "$build_dir" -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" -j --target bench_spsc

# Détection automatique de l'exécutable selon le système
if [[ -f "$build_dir/bench_spsc" ]]; then
    echo "➡️ Running Linux/Mac executable..."
    "$build_dir/bench_spsc"
elif [[ -f "$build_dir/Debug/bench_spsc.exe" ]]; then
    echo "➡️ Running Windows Debug executable..."
    "$build_dir/Debug/bench_spsc.exe"
elif [[ -f "$build_dir/Release/bench_spsc.exe" ]]; then
    echo "➡️ Running Windows Release executable..."
    "$build_dir/Release/bench_spsc.exe"
else
    echo "❌ Error: bench_spsc executable not found!"
    echo "Expected in: $build_dir/ or $build_dir/Debug/ or $build_dir/Release/"
    exit 1
fi

# SPSC Ring Buffer Benchmark (C++20)

**Objectif** : mesurer le débit (ops/s) et la latence (p50/p95/p99) d'un ring buffer **SPSC** (Single-Producer, Single-Consumer) lock-free, avec vérification de correction (pas de perte/duplication).

## Build & Tests

```bash
scripts/run_tests.sh

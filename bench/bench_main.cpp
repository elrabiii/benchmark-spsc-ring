#include "ring_buffer.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <cstdio>
#include <random>
#include <numeric>

using Clock = std::chrono::high_resolution_clock;

struct Args {
    size_t capacity = 1u << 20; // 1M
    int producers = 1;          // SPSC -> 1 producteur
    int consumers = 1;          // SPSC -> 1 consommateur
    int messages  = 5'000'000;
    int warmup    = 200'000;
    unsigned seed = 12345;
};

int main() {
    Args a{};
    SpscRingBuffer<uint32_t> rb(a.capacity);

    // Warm-up
    for (int i = 0; i < a.warmup; ++i) {
        while (!rb.try_push((uint32_t)i)) { /* spin */ }
        while (!rb.try_pop()) { /* spin */ }
    }

    std::mt19937 rng(a.seed);
    std::uniform_int_distribution<uint32_t> dist(0, 1'000'000);

    std::vector<double> lat_samples;
    lat_samples.reserve(20000); // échantillonnage
    const int sample_every = 200;

    auto t0 = Clock::now();

    std::thread prod([&]{
        for (int i = 1; i <= a.messages; ++i) {
            uint32_t v = dist(rng);
            while (!rb.try_push(v)) { /* spin */ }
        }
    });

    std::atomic<int> consumed{0};
    std::thread cons([&]{
        int local = 0;
        while (local < a.messages) {
            auto ts = Clock::now();
            auto v = rb.try_pop();
            if (v) {
                ++local;
                if ((local % sample_every) == 0) {
                    auto te = Clock::now();
                    std::chrono::duration<double, std::micro> us = te - ts;
                    lat_samples.push_back(us.count());
                }
            }
        }
        consumed.store(local, std::memory_order_relaxed);
    });

    prod.join();
    cons.join();
    auto t1 = Clock::now();
    std::chrono::duration<double> sec = t1 - t0;

    double throughput = a.messages / sec.count();

    // Quantiles rudimentaires
    std::sort(lat_samples.begin(), lat_samples.end());
    auto q = [&](double p){
        if (lat_samples.empty()) return 0.0;
        size_t idx = static_cast<size_t>(p * (lat_samples.size() - 1));
        return lat_samples[idx];
    };
    double p50 = q(0.50), p95 = q(0.95), p99 = q(0.99);

    // Rapport JSON minimal (machine-lisible)
    std::printf("{\"messages\":%d,\"seconds\":%.6f,\"throughput_ops\":%.2f,"
                "\"lat_us_p50\":%.3f,\"lat_us_p95\":%.3f,\"lat_us_p99\":%.3f}\n",
                a.messages, sec.count(), throughput, p50, p95, p99);
    return 0;
}

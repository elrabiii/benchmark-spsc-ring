#pragma once
#include <atomic>
#include <cstddef>
#include <vector>
#include <optional>
#include <cassert>

// Single-Producer Single-Consumer bounded ring buffer.
// Hypothèses : un thread producteur, un thread consommateur.
// Pas d'allocation après construction. Taille fixe (power-of-two conseillée).

template <typename T>
class SpscRingBuffer {
public:
    explicit SpscRingBuffer(size_t capacity)
        : capacity_(capacity), mask_(capacity - 1),
          buf_(capacity), head_(0), tail_(0)
    {
        // Pour simplifier, on recommande une capacité puissance de 2 (mask rapide).
        assert(capacity_ >= 2);
        assert((capacity_ & (capacity_ - 1)) == 0 && "capacity must be power of two");
    }

    // Non copyable
    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;

    bool try_push(const T& v) {
        auto head = head_.load(std::memory_order_relaxed);
        auto next = head + 1;
        if (next - tail_.load(std::memory_order_acquire) > capacity_) return false; // full
        buf_[head & mask_] = v;
        head_.store(next, std::memory_order_release);
        return true;
    }

    std::optional<T> try_pop() {
        auto tail = tail_.load(std::memory_order_relaxed);
        if (head_.load(std::memory_order_acquire) == tail) return std::nullopt; // empty
        T v = std::move(buf_[tail & mask_]);
        tail_.store(tail + 1, std::memory_order_release);
        return v;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    bool full() const {
        return (head_.load(std::memory_order_acquire) - tail_.load(std::memory_order_acquire)) > capacity_ - 1;
    }

    size_t size() const {
        return head_.load(std::memory_order_acquire) - tail_.load(std::memory_order_acquire);
    }

    size_t capacity() const { return capacity_; }

private:
    const size_t capacity_;
    const size_t mask_;
    std::vector<T> buf_;
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};

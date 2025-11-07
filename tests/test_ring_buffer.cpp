#include <gtest/gtest.h>

#include <thread>

#include "ring_buffer.hpp"

TEST(SpscRingBuffer, BasicPushPop) {
  SpscRingBuffer<int> rb(1024);
  EXPECT_TRUE(rb.try_push(42));
  auto v = rb.try_pop();
  ASSERT_TRUE(v.has_value());
  EXPECT_EQ(*v, 42);
  EXPECT_TRUE(rb.empty());
}

TEST(SpscRingBuffer, FifoOrderSingleThread) {
  SpscRingBuffer<int> rb(8);
  for (int i = 0; i < 7; ++i) {
    EXPECT_TRUE(rb.try_push(i));
  }
  for (int i = 0; i < 7; ++i) {
    auto v = rb.try_pop();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, i);
  }
  EXPECT_TRUE(rb.empty());
}

TEST(SpscRingBuffer, ProducerConsumerThreads) {
  SpscRingBuffer<int> rb(1024);
  constexpr int N = 100000;
  std::atomic<int> produced{0}, consumed{0}, sum{0};

  std::thread prod([&] {
    for (int i = 1; i <= N;) {
      if (rb.try_push(i)) {
        produced.fetch_add(1, std::memory_order_relaxed);
        ++i;
      }
    }
  });

  std::thread cons([&] {
    int local_sum = 0;
    int c = 0;
    while (c < N) {
      auto v = rb.try_pop();
      if (v) {
        local_sum += *v;
        ++c;
      }
    }
    sum.store(local_sum, std::memory_order_relaxed);
    consumed.store(c, std::memory_order_relaxed);
  });

  prod.join();
  cons.join();

  EXPECT_EQ(produced.load(), N);
  EXPECT_EQ(consumed.load(), N);
  // somme 1..N = N*(N+1)/2
  EXPECT_EQ(sum.load(), N * (N + 1) / 2);
}

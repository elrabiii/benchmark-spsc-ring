cmake_minimum_required(VERSION 3.20)
project(benchmark_spsc_ring LANGUAGES CXX)

option(BUILD_TESTS "Build unit tests" ON)
option(ENABLE_ASAN "Enable Address/UB sanitizers (Debug)" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Flags de perf/repro
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
  endif()
endif()

if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
  endif()
endif()

add_library(ring_buffer
  src/ring_buffer.cpp
)
target_include_directories(ring_buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Bench binaire
add_executable(bench_spsc bench/bench_main.cpp)
target_link_libraries(bench_spsc PRIVATE ring_buffer)

# Tests (GoogleTest via FetchContent)
if(BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_executable(unit_tests tests/test_ring_buffer.cpp)
  target_link_libraries(unit_tests PRIVATE ring_buffer gtest_main)

  # add_test(NAME unit_tests COMMAND unit_tests)
  include(GoogleTest)
  gtest_discover_tests(unit_tests)
endif()

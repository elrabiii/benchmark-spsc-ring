FROM ubuntu:22.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential cmake ninja-build git curl && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /work
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
CMD ["./build/bench_spsc"]

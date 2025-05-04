# Use the official Ubuntu LTS as base image
FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install basic development tools and CMake (new enough to support FetchContent)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    curl \
    tar \
    unzip \
    ninja-build \
    g++ \
    clang \
    lldb \
    lld \
    clang-tidy \
    clang-format \
    && rm -rf /var/lib/apt/lists/*

# Install gRPC build dependencies (needed even with FetchContent)
RUN apt-get update && apt-get install -y \
    libssl-dev \
    libc-ares-dev \
    zlib1g-dev \
    libprotobuf-dev \
    protobuf-compiler \
    && rm -rf /var/lib/apt/lists/*

# Install additional useful tools
RUN apt-get update && apt-get install -y \
    gdb \
    valgrind \
    ccache \
    && rm -rf /var/lib/apt/lists/*


# Set the working directory to your project
WORKDIR /workspace/cc/flwr

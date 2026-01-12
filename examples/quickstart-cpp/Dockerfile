# Copyright 2024 Flower Labs GmbH. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    ca-certificates \
    libssl-dev \
    autoconf \
    automake \
    libtool \
    curl \
    make \
    g++ \
    unzip \
    && rm -rf /var/lib/apt/lists/*

# Build and install Protobuf 3.21.12 using CMake (generates ProtobufConfig.cmake)
RUN cd /tmp && \
    wget https://github.com/protocolbuffers/protobuf/archive/v3.21.12.tar.gz && \
    tar -xzf v3.21.12.tar.gz && \
    cd protobuf-3.21.12 && \
    cmake -S cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_SHARED_LIBS=ON && \
    cmake --build build -j$(nproc) && \
    cmake --install build && \
    ldconfig && \
    cd / && \
    rm -rf /tmp/protobuf-* /tmp/v3.21.12.tar.gz

# Set working directory
WORKDIR /workspace

# Copy the entire project structure
COPY . .

# Remove pre-generated proto files to force regeneration
RUN rm -f framework/cc/flwr/include/flwr/proto/*.pb.cc framework/cc/flwr/include/flwr/proto/*.pb.h

# Build the C++ client with local flwr
RUN cmake -S examples/quickstart-cpp -B build -DUSE_LOCAL_FLWR=ON && \
    cmake --build build

# Default command
CMD ["./build/flwr_client"]

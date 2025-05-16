# Flower C++ bindings

This repository contains the Flower C++ bindings, which integrates gRPC and Protocol Buffers for communication. Below are the installation and build instructions for **Arch Linux** and **Ubuntu**.

## Prerequisites

### Arch Linux
Install the required dependencies using `pacman`:
```bash
sudo pacman -S --needed base-devel cmake git protobuf grpc
```

Check if `grpc_cpp_plugin` is available:
```bash
which grpc_cpp_plugin
```

### Ubuntu (22.04+ recommended)
First, update the package list:
```bash
sudo apt update && sudo apt upgrade -y
```
Install the required dependencies:
```bash
sudo apt install -y build-essential cmake git protobuf-compiler libprotobuf-dev \
    grpc-dev libgrpc++-dev libgrpc-dev
```
If `grpc_cpp_plugin` is missing, build gRPC manually:
```bash
git clone --recurse-submodules -b v1.43.2 https://github.com/grpc/grpc.git
cd grpc
mkdir -p cmake/build && cd cmake/build
cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc)
sudo make install
```

## Building the Project
Once dependencies are installed, build the project:
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

## Running the Application
Once compiled, run the built executable:
```bash
./your_executable_name
```

## Troubleshooting
If errors occur:
1. **Ensure all dependencies are installed**
   ```bash
   which protoc
   which grpc_cpp_plugin
   ```
2. **Manually set Protobuf paths (if necessary)**
   ```bash
   export PATH=/usr/local/bin:$PATH
   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
   ```

If you encounter issues, please report them in the issue tracker.


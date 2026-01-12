cmake_minimum_required(VERSION 3.16)
project(SimpleCppFlowerClient VERSION 0.10
  DESCRIPTION "Creates a Simple C++ Flower client that trains a linear model on synthetic data."
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
option(USE_LOCAL_FLWR "Use local Flower directory instead of fetching from GitHub" OFF)

######################
### Download gRPC 

include(FetchContent)

# Configure gRPC and dependency options
set(gRPC_BUILD_TESTS OFF CACHE BOOL "Disable gRPC tests")
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "Disable C# extension")
set(gRPC_INSTALL OFF CACHE BOOL "Disable gRPC install")
set(protobuf_INSTALL OFF CACHE BOOL "Disable protobuf install")
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests")
set(utf8_range_ENABLE_INSTALL OFF CACHE BOOL "Disable utf8_range install")
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "Disable abseil install")
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "Propagate C++ standard to abseil")
set(gRPC_SSL_PROVIDER module CACHE STRING "Use bundled BoringSSL")

# Force gRPC to use system-installed Protobuf (built in Dockerfile)
set(gRPC_PROTOBUF_PROVIDER "package" CACHE STRING "Use system protobuf")
find_package(Protobuf REQUIRED)

FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.68.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

######################
### FLWR_LIB
if(USE_LOCAL_FLWR)
  # Use add_subdirectory to properly build the framework library
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../framework/cc/flwr ${CMAKE_BINARY_DIR}/flwr)
  set(FLWR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../framework/cc/flwr/include")
else()
  FetchContent_Declare(
    flwr_repo
    GIT_REPOSITORY https://github.com/adap/flower.git
    GIT_TAG        main
  )
  FetchContent_MakeAvailable(flwr_repo)
  add_subdirectory(${flwr_repo_SOURCE_DIR}/framework/cc/flwr ${CMAKE_BINARY_DIR}/flwr)
  set(FLWR_INCLUDE_DIR "${flwr_repo_SOURCE_DIR}/framework/cc/flwr/include")
endif()

######################
### FLWR_CLIENT
file(GLOB FLWR_CLIENT_SRCS src/*.cc)
set(EXECUTABLE_NAME flwr_client)
add_executable(${EXECUTABLE_NAME} ${FLWR_CLIENT_SRCS})

target_include_directories(${EXECUTABLE_NAME} PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  ${FLWR_INCLUDE_DIR}
)

target_link_libraries(${EXECUTABLE_NAME}
  flwr
)

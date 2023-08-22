cmake_minimum_required(VERSION 3.0.0)
project(wasmkeeper VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
include_directories(include)

find_package(jsoncpp CONFIG REQUIRED)

add_executable(wasmkeeper
  main.cpp
  src/wasmkeeper/vm.cpp
  src/wasmkeeper/utils.cpp)

target_link_libraries(wasmkeeper
  PRIVATE wasmedge
  PRIVATE JsonCpp::JsonCpp)

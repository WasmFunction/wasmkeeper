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

add_executable(test
  test.cpp
  src/wasmkeeper/vm.cpp)

target_link_libraries(test
  PRIVATE wasmedge)

add_executable(test2
  test2.cpp)

target_link_libraries(test2
  PRIVATE JsonCpp::JsonCpp)

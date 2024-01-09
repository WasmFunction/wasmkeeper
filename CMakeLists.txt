cmake_minimum_required(VERSION 3.19)
project(wasmkeeper VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(CLI11 CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(wasmkeeper
  main.cpp
  src/wasmkeeper/wasmedgepp.cpp
  src/wasmkeeper/utils.cpp)

target_include_directories(wasmkeeper PRIVATE include)
target_link_libraries(wasmkeeper PRIVATE CLI11::CLI11)
target_link_libraries(wasmkeeper PRIVATE httplib::httplib)
target_link_libraries(wasmkeeper PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(wasmkeeper PRIVATE wasmedge)

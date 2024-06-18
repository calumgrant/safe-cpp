cmake_minimum_required(VERSION 3.7)
project(Safe-cpp)

SET(CMAKE_CXX_STANDARD 20)

enable_testing()

include_directories(include)

add_executable(unittests test/main.cpp)
add_executable(benchmark test/benchmark.cpp)
add_executable(tutorial test/tutorial.cpp)
# add_executable(unsafe test/unsafe.cpp)

add_test(unittests unittests)
add_test(tutorial tutorial)

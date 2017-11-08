cmake_minimum_required(VERSION 3.8)
project(Traceratops CXX)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCE_FILES src/*)
add_executable(Traceratops ${SOURCE_FILES})
target_include_directories(Traceratops PRIVATE deps/)

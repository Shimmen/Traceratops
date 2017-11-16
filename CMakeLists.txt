cmake_minimum_required(VERSION 3.8)
project(Traceratops CXX)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCE_FILES src/*)
add_executable(Traceratops ${SOURCE_FILES})
target_include_directories(Traceratops PRIVATE deps/)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    target_compile_definitions(Traceratops USE_OPENMP)
else()
    message(WARNING "OpenMP not found, reverting to single threaded execution!")
endif()

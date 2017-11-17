cmake_minimum_required(VERSION 3.0)
project(Traceratops CXX)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCE_FILES src/*)
add_executable(Traceratops ${SOURCE_FILES})
target_include_directories(Traceratops PRIVATE deps/)

# For MinGW where it complains that it can't be found but it does find it...
#target_compile_definitions(Traceratops PUBLIC USE_OPENMP)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    target_compile_definitions(Traceratops PUBLIC USE_OPENMP)
else()
    message(WARNING "OpenMP not found, reverting to single threaded execution!")
endif(OPENMP_FOUND)

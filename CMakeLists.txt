cmake_minimum_required(VERSION 3.0)
project(Traceratops CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

file(GLOB SOURCE_FILES src/*)
add_executable(Traceratops ${SOURCE_FILES})
target_include_directories(Traceratops PRIVATE deps/)

# Add minimal GLFW setup
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)
target_link_libraries(Traceratops glfw)

# Add OpenGL (since we don't need any extensions)
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    target_include_directories(Traceratops PRIVATE ${OPENGL_INCLUDE_DIR})
    target_link_libraries(Traceratops ${OPENGL_gl_LIBRARY})
endif ()

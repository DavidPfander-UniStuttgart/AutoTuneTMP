cmake_minimum_required(VERSION 3.2)

project(cppjit)

# find_package(PkgConfig REQUIRED)

include_directories(include)

set(CMAKE_CXX_FLAGS "-std=c++14 -g -Wall -Wextra ${CMAKE_CXX_FLAGS}")

include_directories(include)

#However, the file(GLOB...) allows for wildcard additions:
set(SOURCES_UNROLL_LOOP "examples/unroll_example.cpp")
add_executable(unroll_example ${SOURCES_UNROLL_LOOP})
# target_link_libraries(hello_world_jit dl)

# file(GLOB SOURCES_TEST "test/test_cppjit.cpp")
# add_executable(test_cppjit ${SOURCES_TEST})
# target_link_libraries(test_cppjit dl)


cmake_minimum_required(VERSION 3.2)
project(sol)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DBITS=64 -DNOANSI -DDEBUG")

set(SOURCE_FILES
    codegen.cc
    compiler.hpp
    debug.cc
    debug.hpp
    intrprtr.cc
    intrprtr.hpp
    memory.cc
    memory.hpp
    objects.cc
    objects.hpp
    other.cc
    other.hpp
    parser.cc
    primitiv.cc
    primitiv.hpp
    scanner.cc
    sol.cc
    sol.hpp
    streams.cc
    streams.hpp
    sysdep.cc
    sysdep.hpp)

add_executable(sol ${SOURCE_FILES})

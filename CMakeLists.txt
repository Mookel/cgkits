cmake_minimum_required(VERSION 3.5)
project(cgkits)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    include/debug.h
    include/stack.h
    include/yystack.h
    README.md)

include_directories(include)

add_executable(cgkits ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.16)
project(SkipList)

set(CXX_FLAGS
        -fno-omit-frame-pointer
        -Wall
        -O3
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -std=c++17
        -march=native)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable(SkipList main.cpp SkipList.h)

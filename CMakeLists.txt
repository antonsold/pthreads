cmake_minimum_required(VERSION 3.12)
project(trial)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

add_executable(trial main.cpp)
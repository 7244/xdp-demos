cmake_minimum_required(VERSION 3.10)

project(control_xdp)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)

add_executable(control_xdp.exe control_xdp.cpp)

target_link_libraries(control_xdp.exe bpf)

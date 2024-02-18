cmake_minimum_required(VERSION 3.20)

project(Server)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(server src/main.cpp src/server.cpp)

target_compile_options(server PUBLIC -std=c++20 -Wall)
target_include_directories(server PUBLIC includes/)

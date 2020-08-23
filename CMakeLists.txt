cmake_minimum_required(VERSION 3.9)
project(zcontrol)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(base)

add_executable(zcontrol main.cpp)
target_link_libraries(zcontrol PRIVATE zcontrol-base)

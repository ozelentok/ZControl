cmake_minimum_required(VERSION 3.9)
project(zcontrol)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(base)
add_subdirectory(common)
add_subdirectory(commander)
add_subdirectory(server)
add_subdirectory(worker)

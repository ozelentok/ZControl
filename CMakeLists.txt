cmake_minimum_required(VERSION 3.9)
project(zcontrol)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-D_FILE_OFFSET_BITS=64)

add_subdirectory(base)
add_subdirectory(common)
add_subdirectory(commander)
add_subdirectory(server)
add_subdirectory(zcfs)
add_subdirectory(worker)

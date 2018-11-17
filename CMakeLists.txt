cmake_minimum_required(VERSION 3.8)
project(CraftBone)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -lSDL2")

set(SOURCE_FILES main.cpp)
add_executable(CraftBone ${SOURCE_FILES})
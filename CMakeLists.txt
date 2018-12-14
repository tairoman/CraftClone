cmake_minimum_required(VERSION 3.8)
project(CraftBone)

set(CMAKE_CXX_STANDARD 17)

#find_package(GLEW REQUIRED)
#if (GLEW_FOUND)
#include_directories(${GLEW_INCLUDE_DIRS})
#target_link_libraries(Starting GLEW::GLEW)
#endif()

set(CMAKE_CXX_FLAGS "-Wall -lSDL2 -lGL -lGLEW")

set(SOURCE_FILES main.cpp Engine/WindowManager.cpp Engine/WindowManager.h)
add_executable(CraftBone ${SOURCE_FILES})
#target_link_libraries(CraftBone GLEW)
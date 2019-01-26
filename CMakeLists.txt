cmake_minimum_required(VERSION 3.8)
project(CraftBone)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -lSDL2 -lGL -lGLEW")

set(SOURCE_FILES main.cpp Engine/WindowManager.cpp Engine/WindowManager.h Engine/Shader.cpp Engine/Shader.h Engine/Camera.cpp Engine/Camera.h Engine/Chunk.cpp Engine/Chunk.h)
add_executable(CraftBone ${SOURCE_FILES})
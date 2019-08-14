cmake_minimum_required(VERSION 3.8)
project(CraftBone)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -lSDL2 -lGL -lGLEW")

file(GLOB imgui
        "lib/imgui/*.h"
        "lib/imgui/*.cpp"
        "imgui_impl_sdl_gl3.h"
        "imgui_impl_sdl_gl3.cpp"
        )

set(SOURCE_FILES main.cpp Engine/WindowManager.cpp Engine/WindowManager.h Engine/Shader.cpp Engine/Shader.h Engine/Camera.cpp Engine/Camera.h Engine/Chunk.cpp Engine/Chunk.h Engine/World.cpp Engine/World.h)
add_executable(CraftBone ${SOURCE_FILES} ${imgui})
cmake_minimum_required(VERSION 3.18)
project(CraftBone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wextra -pedantic -lSDL2 -lGL -lGLEW")
endif(CMAKE_COMPILER_IS_GNUCXX)

file(GLOB imgui
        "lib/imgui/*.h"
        "lib/imgui/*.cpp"
        "imgui_impl_sdl_gl3.h"
        "imgui_impl_sdl_gl3.cpp"
        )

find_package(GLEW REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

set(RESOURCE_FILES
        shaders/shader.vert
        shaders/shader.frag
        assets/tiles.png
)

message("Copying resource files")
foreach(X ${RESOURCE_FILES})
        set(ABS_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${X})
        if (EXISTS ${ABS_FILE_PATH})
                message("Copying file ${ABS_FILE_PATH}")
                configure_file(${X} ${X} COPYONLY)
        else()
                message(SEND_ERROR "File not found: ${ABS_FILE_PATH}")
        endif()
endforeach()

add_definitions(-DCRAFTBONE_ENABLE_DEBUG_OPENGL)

set(SOURCE_FILES
        main.cpp
        Engine/WindowManager.cpp
        Engine/WindowManager.h
        Engine/Shader.cpp
        Engine/Shader.h
        Engine/Camera.cpp
        Engine/Camera.h
        Engine/Chunk.cpp
        Engine/Chunk.h
        Engine/ChunkMesh.cpp
        Engine/ChunkMesh.h
        Engine/ChunkManager.cpp
        Engine/ChunkManager.h
        Engine/World.cpp
        Engine/World.h
        Engine/Logger.cpp
        Engine/Logger.h
        Engine/utils/ChunkIndex.cpp
        Engine/utils/ChunkIndex.h
        lib/PerlinNoise.hpp
)

add_executable(CraftBone ${SOURCE_FILES} ${imgui})

if (WIN32)
    set_target_properties(CraftBone PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE)
endif()

source_group("Headers" Engine/WindowManager.h Engine/Shader.h Engine/Camera.h Engine/Chunk.h Engine/World.h Engine/Logger.h)
source_group("Source" Engine/WindowManager.cpp Engine/Shader.cpp Engine/Camera.cpp Engine/Chunk.cpp Engine/World.cpp Engine/Logger.cpp)

if (WIN32)
    target_link_libraries(CraftBone SDL2::SDL2 GLEW::GLEW glm::glm OpenGL::GL)
else()
    target_link_libraries(CraftBone SDL2::SDL2 GLEW::GLEW glm OpenGL::GL)
endif()
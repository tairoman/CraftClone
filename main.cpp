#include <iostream>
#include <fstream>
#include <memory>
#include <array>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#include "lib/imgui/imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "Engine/WindowManager.h"
#include "Engine/Shader.h"
#include "Engine/Chunk.h"
#include "Engine/Camera.h"
#include "Engine/World.h"
#include "Engine/Logger.h"
#include "Engine/utils/Chunkindex.h"

struct Config
{
    bool wireframe = false;
    bool vsync = false;
    bool fullscreen = false;
};

struct Stats {
    std::size_t currentFPS = 0;
};

namespace
{

//TODO: Remove globals
Stats stats;
Config config;
Engine::Camera* playerCamera;

bool showingConfig = false;

auto& s_windowManager = Engine::WindowManager::instance();

} // anon namespace

void renderImGui()
{
    ImGui_ImplSdlGL3_NewFrame(s_windowManager.sdlWindow());

    if (showingConfig) {
        ImGui::Begin("Config");
    
        if (ImGui::Checkbox("Wireframe Mode", &config.wireframe)) {
            glPolygonMode(GL_FRONT_AND_BACK, config.wireframe ? GL_LINE : GL_FILL);
        }

        if (ImGui::Checkbox("VSync", &config.vsync)) {
            Engine::WindowManager::instance().setVSync(config.vsync);
        }

        if (ImGui::Checkbox("Fullscreen", &config.fullscreen)) {
            Engine::WindowManager::instance().setWindowMode(
                config.fullscreen ? Engine::WindowMode::Fullscreen : Engine::WindowMode::Windowed
            );
        }

        ImGui::End();
    }

    ImGui::SetNextWindowPos({float(s_windowManager.width()) * 0.85f, float(s_windowManager.height()) * 0.05f});
    ImGui::SetNextWindowSize({float(s_windowManager.width()) * 0.15f, 300});
    ImGui::Begin("Info");
    auto fpsText = std::string("FPS: ") + std::to_string(stats.currentFPS);
    ImGui::Text("%s", fpsText.c_str());

    const auto posXText = std::string("x:") + std::to_string(playerCamera->getPosition().x);
    const auto posYText = std::string("y:") + std::to_string(playerCamera->getPosition().y);
    const auto posZText = std::string("z:") + std::to_string(playerCamera->getPosition().z);
    ImGui::Text("%s", posXText.c_str());
    ImGui::Text("%s", posYText.c_str());
    ImGui::Text("%s", posZText.c_str());

    ImGui::End();

    ImGui::Render();
}

int main(int argc, char* argv[])
{

#if defined (CRAFTBONE_ENABLE_DEBUG_OPENGL)
    //Logger::registerGlLogger();
#endif

    SDL_Window* window = Engine::WindowManager::instance().sdlWindow();

    ImGui_ImplSdlGL3_Init(window);

    int textureAtlasWidth = -1;
    int textureAtlasHeight = -1;

    stbi_set_flip_vertically_on_load((int) true);
    const auto tilesFile = "assets/tiles.png";
    int comp = -1;
    std::unique_ptr<stbi_uc> image{ stbi_load(tilesFile, &textureAtlasWidth, &textureAtlasHeight, &comp, STBI_rgb_alpha) };

    if (image == nullptr || textureAtlasWidth < 0 || textureAtlasHeight < 0) {
        std::cerr << tilesFile << " could not be loaded.\n";
        exit(0);
    }

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlasWidth, textureAtlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.get());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16);

    glGenerateMipmap(GL_TEXTURE_2D);

    bool running = true;

    SDL_Event event{};

    const Engine::Shader simpleShader("shaders/shader.vert", "shaders/shader.frag");

    std::array<float,3> backgroundColor{0.2f, 0.2f, 0.8f};

    Engine::Camera camera(45.0f, float(s_windowManager.width()) / float(s_windowManager.height()), 0.01f, 500.0f);
    camera.setPosition(glm::vec3(1000.0f, 100.0f, 1000.0f));
    playerCamera = &camera;

    auto world = Engine::World{texture};

    auto now = SDL_GetTicks();
    double avgDeltaTime = 1;

    auto currentChunkIndex = ChunkIndex::fromWorldPos(playerCamera->getPosition());
    world.setPlayerChunk(currentChunkIndex);

    while (running) {

        const auto last = now;
        now = SDL_GetTicks();

        const auto deltaTime = now - last;

        // Moving average of time spent in game loop
        const auto decayFactor = 0.95;
        avgDeltaTime = decayFactor * avgDeltaTime + (1.0 - decayFactor) * deltaTime;
		
        // Display calculated FPS of the moving average
        stats.currentFPS = static_cast<std::size_t>(std::round(1000 / avgDeltaTime));

        if (!showingConfig) {
            camera.update();
        }

        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        simpleShader.use();
        world.render(camera.getPosition(), simpleShader, camera.getProjection() * camera.getView());

        glUseProgram( 0 );

        renderImGui();

        SDL_GL_SwapWindow(window);

        while (SDL_PollEvent(&event) == 1) {

            ImGui_ImplSdlGL3_ProcessEvent(&event);

            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE && showingConfig) {
                showingConfig = false;
            }
            else if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
            } else if (event.type == SDL_KEYUP) {

                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        showingConfig = !showingConfig;
                        SDL_SetRelativeMouseMode(showingConfig ? SDL_FALSE : SDL_TRUE);
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_MOUSEMOTION && !showingConfig) {
                camera.rotate(event.motion.xrel, event.motion.yrel);
            }
        }

        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

        if (!showingConfig) {
            if (keyState[SDL_SCANCODE_W]) {
                camera.moveForward(deltaTime / 100.0f);
            }
            if (keyState[SDL_SCANCODE_S]) {
                camera.moveBack(deltaTime / 100.0f);
            }
            if (keyState[SDL_SCANCODE_A]) {
                camera.moveLeft(deltaTime / 100.0f);
            }
            if (keyState[SDL_SCANCODE_D]) {
                camera.moveRight(deltaTime / 100.0f);
            }
            if (keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(10);
            }
            if (!keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(1);
            }
        }

        auto newChunkIndex = ChunkIndex::fromWorldPos(playerCamera->getPosition());
        if (currentChunkIndex.data() != newChunkIndex.data()) {
            currentChunkIndex = newChunkIndex;
            world.setPlayerChunk(currentChunkIndex);
        }
    }

    SDL_Quit();

    return 0;
}
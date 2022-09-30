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
#include "Engine/utils/Observer.h"
#include "Engine/GameEventDispatcher.h"
#include "Engine/FpsCounter.h"

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

    const auto& playerPos = playerCamera->position.get();
    const auto posXText = std::string("x:") + std::to_string(playerPos.x);
    const auto posYText = std::string("y:") + std::to_string(playerPos.y);
    const auto posZText = std::string("z:") + std::to_string(playerPos.z);
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

    std::cout << "Running main thread with id: " << std::this_thread::get_id() << "\n";

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
    camera.position.set(glm::vec3(1000.0f, 100.0f, 1000.0f));
    playerCamera = &camera;

    auto world = Engine::World{texture, playerCamera};

    GameEventDispatcher gameEvents;
    FpsCounter fpsCounter{ gameEvents };

    Observer m_observer;

    fpsCounter.onFpsChanged.listen(m_observer, [](const auto& deltaTime) {
        stats.currentFPS = deltaTime;
    });

    gameEvents.onNewFrame.listen(m_observer, [&camera](const auto& deltaTime) {
        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

        if (!showingConfig) {
            auto moveVec = glm::vec2{ 0,0 };
            if (keyState[SDL_SCANCODE_W]) {
                moveVec.y += 1;
            }
            if (keyState[SDL_SCANCODE_S]) {
                moveVec.y -= 1;
            }
            if (keyState[SDL_SCANCODE_A]) {
                moveVec.x -= 1;
            }
            if (keyState[SDL_SCANCODE_D]) {
                moveVec.x += 1;
            }
            if (keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(10);
            }
            if (!keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(1);
            }
            const auto vecNorm = (glm::length(moveVec) > 0 ? glm::normalize(moveVec) : moveVec);
            camera.move(deltaTime / 100.0f * vecNorm);
        }
    });

    while (running) {

        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        simpleShader.use();
        world.render(camera.position.get(), simpleShader, camera.getProjection() * camera.getView());

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

        gameEvents.processLoop();
    }

    SDL_Quit();

    return 0;
}
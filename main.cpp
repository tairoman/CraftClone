#include <iostream>
#include <fstream>
#include <memory>
#include <array>

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

namespace ScreenData
{
    constexpr auto width = 1920;
    constexpr auto height = 1080;
}

namespace Engine
{
    void initialize()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Couldn't initialize SDL: " << SDL_GetError() << ".\n";
            exit(0);
        }

        SDL_GL_LoadLibrary(nullptr);

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    }
}

struct Config
{
    bool wireframe = false;
    bool vsync = false;
};

struct Stats {
    std::size_t currentFPS = 0;
};

static Stats stats;
static Config config;

static bool showingConfig = false;

void renderImGui(Engine::WindowManager* manager)
{
    ImGui_ImplSdlGL3_NewFrame(manager->getWindow());

    if (showingConfig) {
        ImGui::Begin("Config");
    
        if (ImGui::Checkbox("Wireframe Mode", &config.wireframe)) {
            glPolygonMode(GL_FRONT_AND_BACK, config.wireframe ? GL_LINE : GL_FILL);
        }

        if (ImGui::Checkbox("VSync", &config.vsync)) {
            manager->setVSync(config.vsync);
        }

        ImGui::End();
    }

    ImGui::SetNextWindowPos({ScreenData::width * 0.85, ScreenData::height * 0.05});
    ImGui::SetNextWindowSize({ScreenData::width * 0.15, 0});
    ImGui::Begin("Info");
    auto fpsText = std::string("FPS: ") + std::to_string(stats.currentFPS);
    ImGui::Text(fpsText.c_str());

    ImGui::End();

    ImGui::Render();
}

int main()
{
    Engine::initialize();

    Engine::WindowManager windowManager("Test", ScreenData::width, ScreenData::height);
    SDL_Window* window = windowManager.getWindow();

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

    glGenerateMipmap(GL_TEXTURE_2D);

    bool running = true;

    SDL_Event event{};

    int w = 0; 
    int h = 0;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h); // Set viewport

    const Engine::Shader simpleShader("shaders/shader.vert", "shaders/shader.frag");

    std::array<float,3> backgroundColor{0.2f, 0.2f, 0.8f};

    Engine::Camera camera(45.0f, float(w) / float(h), 0.01f, 500.0f);
    camera.setPosition(glm::vec3(-1.0f, 10.0f, 3.0f));

    glm::ivec3 viewDistanceInChunks{2, 2, 2};
    auto world = Engine::World{viewDistanceInChunks, texture};

    auto now = SDL_GetTicks();
    double deltaTime = 0;
    double avgDeltaTime = 1;

    while (running) {

        const auto last = now;
        now = SDL_GetTicks();

        deltaTime = now - last;

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

        renderImGui(&windowManager);

        SDL_GL_SwapWindow(window);

        while (SDL_PollEvent(&event) == 1) {

            ImGui_ImplSdlGL3_ProcessEvent(&event);

            if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
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
                camera.moveForward(deltaTime / 100);
            }
            if (keyState[SDL_SCANCODE_S]) {
                camera.moveBack(deltaTime / 100);
            }
            if (keyState[SDL_SCANCODE_A]) {
                camera.moveLeft(deltaTime / 100);
            }
            if (keyState[SDL_SCANCODE_D]) {
                camera.moveRight(deltaTime / 100);
            }
            if (keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(10);
            }
            if (!keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(1);
            }
        }

    }

    SDL_Quit();

    return 0;
}
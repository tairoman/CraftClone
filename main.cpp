#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#include "lib/imgui/imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "Engine/WindowManager.h"
#include "Engine/Shader.h"
#include "Engine/Chunk.h"
#include "Engine/Camera.h"


const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

struct Config {
    bool wireframe = false;
};

void renderConfig(SDL_Window* window, Config& config){
    ImGui_ImplSdlGL3_NewFrame(window);
    ImGui::Text("Configuration");
    ImGui::Checkbox("Wireframe Mode", &config.wireframe);

    ImGui::Render();

}

#define NUM_BLOCK_TYPES 4

float texLookup[NUM_BLOCK_TYPES*12] = {

        /* 0. GRASS SIDE */
        0.635f, 0.9375f,
        0.759f, 0.9375f,
        0.635f, 1.0f,
        0.635f, 1.0f,
        0.759f, 0.9375f,
        0.759f, 1.0f,

        /* 1. GRASS TOP */

        0.507f, 0.557f,
        0.633f, 0.557f,
        0.507f, 0.619f,
        0.507f, 0.619f,
        0.633f, 0.557f,
        0.633f, 0.619f,

        /* 2. DIRT */

        0.634f, 0.875f,
        0.759f, 0.875f,
        0.634f, 0.936f,
        0.634f, 0.936f,
        0.759f, 0.875f,
        0.759f, 0.936f,

        /* 3. STONE */

        0.254f, 0.62f,
        0.379f, 0.62f,
        0.254f, 0.683f,
        0.254f, 0.683f,
        0.379f, 0.62f,
        0.379f, 0.683f,
};


int main() {

    Engine::WindowManager windowManager("Test", SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_Window* window = windowManager.getWindow();

    ImGui_ImplSdlGL3_Init(window);

    Config config{};

    GLuint texture;
    int textureAtlasWidth, textureAtlasHeight, comp;
    stbi_set_flip_vertically_on_load(true);
    auto tilesFile = "assets/tiles.png";
    unsigned char* image = stbi_load(tilesFile, &textureAtlasWidth, &textureAtlasHeight, &comp, STBI_rgb_alpha);

    if (image == NULL) {
        std::cerr << tilesFile << " could not be loaded.\n";
        exit(0);
    } 

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlasWidth, textureAtlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    free(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    bool running = true;
    bool showConfig = false;

    SDL_Event event{};

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h); // Set viewport

    Engine::Shader simpleShader("shaders/shader.vert", "shaders/shader.frag");

    float backgroundColor[] = {0.2f, 0.2f, 0.8f};

    Engine::Camera camera(45.0f, float(w) / float(h), 0.01f, 500.0f);
    camera.setPosition(glm::vec3(-1.0f, 0.0f, 3.0f));

    Engine::Chunk* chunk = new Engine::Chunk(glm::vec3(0.0f, -128.0, 0.0f), texture);
    Engine::Chunk* chunk1 = new Engine::Chunk(glm::vec3(0.0f, -128.0, 16.0f), texture);
    for (int i = 0; i < BLK_SIZE_X; i++) {
        for (int k = 0; k < BLK_SIZE_Z; k++){
            chunk->set(i, BLK_SIZE_Y - 1, k, Engine::BlockType::GRASS);
            chunk->set(i, BLK_SIZE_Y - 2, k, Engine::BlockType::DIRT);
            chunk->set(i, BLK_SIZE_Y - 3, k, Engine::BlockType::DIRT);
        }
    }

    for (int i = 0; i < BLK_SIZE_X; i++){
        for (int j = 0; j < BLK_SIZE_Y - 3; j++){
            for (int k = 0; k < BLK_SIZE_Z; k++){
                chunk->set(i, j, k, Engine::BlockType::STONE);
                chunk1->set(i, j, k, Engine::BlockType::DIRT);
            }
        }
    }

    while (running) {

        if (!showConfig) camera.update();

        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        simpleShader.use();

        simpleShader.setUniform("modelViewProjectionMatrix", camera.getProjection() * camera.getView() * chunk->modelWorldMatrix);
        simpleShader.setUniform("texLookup", texLookup, NUM_BLOCK_TYPES*12);
        chunk->render();

        simpleShader.setUniform("modelViewProjectionMatrix", camera.getProjection() * camera.getView() * chunk1->modelWorldMatrix);
        simpleShader.setUniform("texLookup", texLookup, NUM_BLOCK_TYPES*12);
        chunk1->render();

        glUseProgram( 0 );

        if (showConfig)
            renderConfig(window, config);

        SDL_GL_SwapWindow(window);

        while (SDL_PollEvent(&event)) {

            ImGui_ImplSdlGL3_ProcessEvent(&event);

            if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
            } else if (event.type == SDL_KEYUP) {

                switch (event.key.keysym.sym) {

                    case SDLK_0:
                        backgroundColor[0] = 0.8f;
                        backgroundColor[1] = 0.2f;
                        backgroundColor[2] = 0.2f;
                        break;
                    case SDLK_1:
                        backgroundColor[0] = 0.2f;
                        backgroundColor[1] = 0.8f;
                        backgroundColor[2] = 0.2f;
                        break;
                    case SDLK_2:
                        backgroundColor[0] = 0.2f;
                        backgroundColor[1] = 0.2f;
                        backgroundColor[2] = 0.8f;
                        break;
                    case SDLK_q:
                        showConfig = !showConfig;
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                camera.rotate(event.motion.xrel, event.motion.yrel);
            }
        }

        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

        if (!showConfig) {
            if (keyState[SDL_SCANCODE_W]) {
                camera.moveForward();
            }
            if (keyState[SDL_SCANCODE_S]) {
                camera.moveBack();
            }
            if (keyState[SDL_SCANCODE_A]) {
                camera.moveLeft();
            }
            if (keyState[SDL_SCANCODE_D]) {
                camera.moveRight();
            }
            if (keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(10);
            }
            if (!keyState[SDL_SCANCODE_LSHIFT]) {
                camera.setSpeedMultiplier(1);
            }
        }

        if (config.wireframe){
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        } else {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
    }

    SDL_Quit();

    return 0;
}
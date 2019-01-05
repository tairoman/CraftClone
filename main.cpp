#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <fstream>

#include "Engine/WindowManager.h"
#include "Engine/Shader.h"
#include "Engine/Camera.h"

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

int main() {

    Engine::WindowManager windowManager("Test", SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_Window* window = windowManager.getWindow();

    bool running = true;

    SDL_Event event{};

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h); // Set viewport

    float vertices[] = {
            //	 X      Y     Z
            0.0f,   0.5f, 1.0f,		// v0
            -0.5f,  -0.5f, 1.0f,	// v1
            0.5f,  -0.5f, 1.0f		// v2
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    Engine::Shader simpleShader("../shader.vert", "../shader.frag");

    GLuint vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);

    float backgroundColor[] = {0.2f, 0.2f, 0.8f};

    Engine::Camera camera(45.0f, float(w) / float(h), 0.01f, 500.0f);
    camera.setPosition(glm::vec3(0.0f, 0.0f, 3.0f));

    while (running) {

        camera.update();

        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE); // Disable backface culling atm

        simpleShader.use();
        simpleShader.setUniform("modelViewProjectionMatrix", camera.getProjection() * camera.getView());

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);


        while (SDL_PollEvent(&event)) {

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
                    default:
                        break;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                static int prev_xcoord = event.motion.x;
                static int prev_ycoord = event.motion.y;
                int delta_x = event.motion.x - prev_xcoord;
                int delta_y = event.motion.y - prev_ycoord;

                //if (event.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                camera.rotate(delta_x, delta_y);
                prev_xcoord = event.motion.x;
                prev_ycoord = event.motion.y;
            }
        }

        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);

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

        glUseProgram( 0 );

    }
    std::cout << "Hello, World!" << std::endl;

    SDL_Quit();

    return 0;
}
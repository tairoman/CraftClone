#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <fstream>

#include "Engine/WindowManager.h"

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

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::ifstream vs_file("../shader.vert");
    std::string vs_src((std::istreambuf_iterator<char>(vs_file)), std::istreambuf_iterator<char>());

    std::ifstream fs_file("../shader.frag");
    std::string fs_src((std::istreambuf_iterator<char>(fs_file)), std::istreambuf_iterator<char>());

    const char *vs = vs_src.c_str();
    const char *fs = fs_src.c_str();

    glShaderSource(vertexShader, 1, &vs, nullptr);
    glShaderSource(fragmentShader, 1, &fs, nullptr);

    glCompileShader(vertexShader);
    int compileOK;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK);
    if(!compileOK) {
        exit(1);
    }

    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK);
    if(!compileOK) {
        exit(2);
    }


    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, vertexShader);

    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );

    glLinkProgram(shaderProgram);

    GLuint vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);

    float backgroundColor[] = {0.2f, 0.2f, 0.8f};

    while (running) {

        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE); // Disable backface culling atm

        glUseProgram( shaderProgram );

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);

        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {

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

                case SDLK_ESCAPE:
                    running = false;
                    break;

                default:
                    break;
            }

        } else if (event.type == SDL_WINDOWEVENT){
            switch (event.window.event){

                //case SDL_WINDOWEVENT_RESIZED:
                //    std::cout << "Resized" << std::endl; break;

                default: break;
            }
        }

        glUseProgram( 0 );

    }
    std::cout << "Hello, World!" << std::endl;

    SDL_Quit();

    return 0;
}
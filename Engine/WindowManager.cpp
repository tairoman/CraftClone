

#include "WindowManager.h"

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

namespace Engine {

WindowManager::WindowManager(const std::string& title) {
    this->width = 640; this->height = 480;
    this->initialize(title);

    //this->setSize(640, 480);

}

WindowManager::WindowManager(const std::string& title, int w, int h) {
    this->width = w; this->height = h;
    this->initialize(title);

    //this->setSize(w, h);

}

void WindowManager::initialize(const std::string& title){

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "%s: %s\n", "Couldn't initialize SDL", SDL_GetError());
        exit(0);
    }

    SDL_GL_LoadLibrary(nullptr);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    this->window = SDL_CreateWindow(title.c_str(),
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    this->width,
                                    this->height,
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (this->window == nullptr) {
        fprintf(stderr, "%s: %s\n", "Couldn't set video mode", SDL_GetError());
        exit(0);
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_WarpMouseInWindow(this->window, this->width / 2, this->height / 2);

    // Create OpenGL context
    this->context = SDL_GL_CreateContext(this->window);
    if (this->context == nullptr) {
        fprintf(stderr, "%s: %s\n", "Failed to create OpenGL context", SDL_GetError());
        exit(0);
    }

    SDL_GL_MakeCurrent(this->window, this->context);

    if (glewInit() != GLEW_OK){
        fprintf(stderr, "%s\n", "Failed to initialize GLEW");
        exit(0);
    }

    // V-SYNC
    SDL_GL_SetSwapInterval(1);

}

WindowManager::~WindowManager() {
    SDL_DestroyWindow(this->window);
}

void WindowManager::setSize(int w, int h) {
    this->width  = w;
    this->height = h;
    SDL_SetWindowSize(this->window, w, h);
}

void WindowManager::setFullscreen() {
    SDL_SetWindowFullscreen(this->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

SDL_Window *WindowManager::getWindow() const {
    return this->window;
}


}
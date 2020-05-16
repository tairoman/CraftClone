#include <iostream>

#include "WindowManager.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

namespace Engine {

WindowManager::WindowManager()
    : m_window(SDL_CreateWindow(
        "",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        0,
        0,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE), SDL_DestroyWindow)
{
    if (m_window == nullptr) {
        std::cerr << "Couldn't set video mode: " << SDL_GetError() << ".\n";
        exit(0);
    }
    
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Create OpenGL context
    m_context = SDL_GL_CreateContext(m_window.get());
    if (m_context == nullptr) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << ".\n";
        exit(0);
    }

    SDL_GL_MakeCurrent(m_window.get(), m_context);

    if (glewInit() != GLEW_OK){
        std::cerr << "Failed to initialize GLEW.\n";
        exit(0);
    }

    setVSync(false);
}

void WindowManager::setSize(int w, int h)
{
    m_width  = w;
    m_height = h;
    SDL_SetWindowSize(m_window.get(), w, h);
    SDL_WarpMouseInWindow(m_window.get(), m_width / 2, m_height / 2);
}

void WindowManager::setFullscreen()
{
    SDL_SetWindowFullscreen(m_window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
}

SDL_Window* WindowManager::getWindow() const
{
    return m_window.get();
}

void WindowManager::setVSync(bool setEnabled)
{
    SDL_GL_SetSwapInterval(static_cast<int>(setEnabled));
}
}
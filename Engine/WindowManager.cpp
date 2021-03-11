#include <SDL2/SDL_video.h>
#include <cstdint>
#include <iostream>

#include "WindowManager.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

namespace
{

using namespace Engine;

std::uint32_t windowModeToSDL(WindowMode mode)
{
    switch (mode) {
    case WindowMode::Windowed:
        return 0;
    case WindowMode::Fullscreen:
        return SDL_WINDOW_FULLSCREEN;
    case WindowMode::WindowedFullscreen:
        return SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
}

}

namespace Engine {

WindowManager::WindowManager()
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
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    m_window.reset(SDL_CreateWindow(
        "",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        0,
        0,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED));

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

    glewExperimental = GL_TRUE;
    const auto init_res = glewInit();
    if(init_res != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW!\n";
        std::cerr << glewGetErrorString(glewInit()) << "\n";
        exit(1);
    }

    if (SDL_GetDesktopDisplayMode(0, &m_currentDisplayMode) != 0) {
        std::cerr << SDL_GetError() << "\n";
        exit(0);
    }

    setSize(m_currentDisplayMode.w, m_currentDisplayMode.h);

    setVSync(false);
}

void WindowManager::setSize(int w, int h)
{
    m_width  = w;
    m_height = h;

    if (m_windowMode == WindowMode::Fullscreen) {
        m_currentDisplayMode.w = m_width;
        m_currentDisplayMode.h = m_height;
        SDL_SetWindowDisplayMode(m_window.get(), &m_currentDisplayMode);
    }
    else {
        SDL_SetWindowSize(m_window.get(), m_width, m_height);
    }

    SDL_WarpMouseInWindow(m_window.get(), m_width / 2, m_height / 2);

    glViewport(0, 0, m_width, m_height); // Set viewport

    std::cout << "New width: " << m_width << "\n";
    std::cout << "New height: " << m_height << "\n";
}

void WindowManager::setWindowMode(WindowMode mode)
{
    m_windowMode = mode;

    SDL_SetWindowFullscreen(m_window.get(), windowModeToSDL(mode));
    if (mode == WindowMode::Fullscreen) {
        setSize(m_width, m_height);
    }
}

SDL_Window* WindowManager::sdlWindow() const
{
    return m_window.get();
}

int WindowManager::width() const
{
    return m_width;
}

int WindowManager::height() const
{
    return m_height;
}

void WindowManager::setVSync(bool setEnabled)
{
    SDL_GL_SetSwapInterval(static_cast<int>(setEnabled));
}
}
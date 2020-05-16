
#ifndef CRAFTBONE_WINDOWMANAGER_H
#define CRAFTBONE_WINDOWMANAGER_H

#include <memory>

#include <SDL2/SDL.h>

namespace Engine
{

class WindowManager
{

public:

    static WindowManager& instance()
    {
        static WindowManager manager;
        return manager;
    }

    void setSize(int w, int h);
    void setFullscreen();
    void setVSync(bool setEnabled);

    [[nodiscard]] SDL_Window* getWindow() const;

private:
    
    WindowManager();

    int m_width = 0;
    int m_height = 0;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window;

    SDL_GLContext m_context;

    Uint32 m_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
};
}


#endif //CRAFTBONE_WINDOWMANAGER_H

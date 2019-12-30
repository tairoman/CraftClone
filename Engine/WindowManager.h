
#ifndef CRAFTBONE_WINDOWMANAGER_H
#define CRAFTBONE_WINDOWMANAGER_H

#include <string>
#include <memory>

#include <SDL2/SDL.h>

#include "SDL/WindowPtr.h"

namespace Engine
{

class WindowManager
    {

    public:

        /* Constructors */
        explicit WindowManager(const std::string& title);
        WindowManager(const std::string& title, int w, int h);

        /* Destructor */
        ~WindowManager() = default;

        /* Set window properties */
        void setSize(int w, int h);
        void setFullscreen();
        void setVSync(bool setEnabled);

        /* Getters */
        SDL_Window* getWindow() const;

    private:

        int width = 0;
        int height = 0;

        SDL::WindowPtr window;

        SDL_GLContext context;

        Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    };
}


#endif //CRAFTBONE_WINDOWMANAGER_H

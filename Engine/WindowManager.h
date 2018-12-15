
#ifndef CRAFTBONE_WINDOWMANAGER_H
#define CRAFTBONE_WINDOWMANAGER_H

#include <SDL2/SDL.h>

#include <string>

namespace Engine {


class WindowManager {

public:

    /* Constructors */
    explicit WindowManager(const std::string& title);
    WindowManager(const std::string& title, int w, int h);

    /* Destructor */
    ~WindowManager();

    /* Set window properties */
    void setSize(int w, int h);
    void setFullscreen();

    /* Getters */
    SDL_Window* getWindow() const;

private:

    void initialize(const std::string& title);

    SDL_Window* window;

    SDL_GLContext context;

    int width = 0;
    int height = 0;

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

};

}


#endif //CRAFTBONE_WINDOWMANAGER_H

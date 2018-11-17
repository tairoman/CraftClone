#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

int main() {
    SDL_Window* window = nullptr;

    SDL_Surface* screenSurface = nullptr;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) { printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError()); exit(0);}

    window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( window == nullptr ) { printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError()); exit(0); }


    screenSurface = SDL_GetWindowSurface( window );

    bool running = true;

    SDL_Event event;

    Uint8 r = 0xFF; Uint8 g = 0xFF; Uint8 b = 0xFF;

    while (running) {

        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT) {
            running = false;
        }

        else if (event.type == SDL_KEYDOWN) {

            switch (event.key.keysym.sym) {

                case SDLK_0: r = 0xFF; g = 0x00; b = 0x00; break;
                case SDLK_1: r = 0x00; g = 0xFF; b = 0x00; break;
                case SDLK_2: r = 0x00; g = 0x00; b = 0xFF; break;

                default: break;
            }

        }

        SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, r, g, b));
        SDL_UpdateWindowSurface(window);
    }
    std::cout << "Hello, World!" << std::endl;

    SDL_DestroyWindow( window );
    SDL_Quit();
    return 0;

    return 0;
}
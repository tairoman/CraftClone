
#ifndef CRAFTBONE_SDL_WINDOWPTR_H
#define CRAFTBONE_SDL_WINDOWPTR_H

#include <memory>
#include <SDL2/SDL.h>

namespace SDL
{

    class WindowPtr
    {
    public:
        
        WindowPtr() = default;
        WindowPtr(SDL_Window* ptr)
        {
            this->ptr.reset(ptr);
        }

        ~WindowPtr() = default;

        decltype(auto) reset(SDL_Window* newPtr)
        {
            return ptr.reset(newPtr);
        }

        [[nodiscard]]
        decltype(auto) get() const
        {
            return ptr.get();
        }

        bool operator==(SDL_Window* otherPtr) const
        {
            return ptr.get() == otherPtr;
        }

    private:
        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> ptr{nullptr, SDL_DestroyWindow};
    };

}

#endif
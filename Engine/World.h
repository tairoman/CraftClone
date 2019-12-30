
#ifndef CRAFTBONE_WORLD_H
#define CRAFTBONE_WORLD_H

#include <memory>
#include "Chunk.h"

constexpr auto WORLD_SIZE_X = 256;
constexpr auto WORLD_SIZE_Y = 16;
constexpr auto WORLD_SIZE_Z = 256;

namespace Engine
{
    class World
    {

        explicit World(glm::vec3 origin);
        ~World() = default;

    private:
        std::unique_ptr<Chunk> chunks[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z];

    };
}


#endif //CRAFTBONE_WORLD_H

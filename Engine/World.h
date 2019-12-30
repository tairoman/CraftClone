//
// Created by marcus on 2019-08-14.
//

#ifndef CRAFTBONE_WORLD_H
#define CRAFTBONE_WORLD_H

#include <memory>
#include "Chunk.h"

#define WORLD_SIZE_X 256
#define WORLD_SIZE_Y 16
#define WORLD_SIZE_Z 256

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


#ifndef CRAFTBONE_WORLD_H
#define CRAFTBONE_WORLD_H

#include <memory>
#include <vector>
#include "Chunk.h"
#include "Shader.h"

namespace Engine
{
    class World
    {
    public:
        World(glm::ivec3 viewDistanceInChunks, GLuint texture);
        ~World() = default;

        void render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix);
        //Engine::BlockType get(int x, int y, int z) const;

    private:

        void renderChunks(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix);
        bool isWithinViewDistance(Chunk* chunk, const glm::vec3& playerPos) const;

        std::vector<std::unique_ptr<Chunk>> chunks;
        glm::vec3 viewDistance;

    };
}


#endif //CRAFTBONE_WORLD_H

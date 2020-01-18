

#include <GL/glew.h>

#include "World.h"


namespace Engine
{

    static glm::vec3 convertViewDistance(glm::ivec3 viewDistanceInChunks)
    {
        // Converts view distance in the form of number of chunks in every direction to actual world length
        return glm::vec3
        {
            viewDistanceInChunks.x * ChunkData::BLOCKS_X * ChunkData::BLOCK_WORLD_EXTENT,
            viewDistanceInChunks.y * ChunkData::BLOCKS_Y * ChunkData::BLOCK_WORLD_EXTENT,
            viewDistanceInChunks.z * ChunkData::BLOCKS_Z * ChunkData::BLOCK_WORLD_EXTENT
        };
    }

    World::World(glm::ivec3 viewDistanceInChunks, GLuint texture)
        : viewDistance(convertViewDistance(viewDistanceInChunks))
    {
        chunks.reserve(viewDistanceInChunks.x * viewDistanceInChunks.y * viewDistanceInChunks.z);
        for (auto x = -viewDistanceInChunks.x; x < viewDistanceInChunks.x; x++)
        {
            for (auto y = -viewDistanceInChunks.y; y < viewDistanceInChunks.y; y++)
            {
                for (auto z = -viewDistanceInChunks.z; z < viewDistanceInChunks.z; z++)
                {
                    chunks.emplace_back(std::make_unique<Chunk>(convertViewDistance(glm::ivec3{x,y,z}), texture, y < 0 ? BlockType::DIRT : BlockType::AIR));
                }
            }
        }
    }

    /*BlockType World::get(int x, int y, int z) const
    {
        //TODO
    }*/

    void World::render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix)
    {
        renderChunks(playerPos, shader, viewProjectionMatrix);
    }

    void World::renderChunks(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix)
    {
        for (const auto& chunk : chunks)
        {
            if (isWithinViewDistance(chunk.get(), playerPos))
            {
                shader.setUniform("modelViewProjectionMatrix", viewProjectionMatrix * chunk->getModelWorldMatrix());
                chunk->render();
            }
            else
            {
                // Don't render and tag for replacement
            }
            
        }
    }

    bool World::isWithinViewDistance(Chunk* chunk, const glm::vec3& playerPos) const
    {
        // Get chunk position relative to player
        const auto chunkPos = chunk->getCenterPos() - playerPos;
        return (
            std::abs(viewDistance.x - chunkPos.x) > 0 &&
            std::abs(viewDistance.y - chunkPos.y) > 0 &&
            std::abs(viewDistance.z - chunkPos.z) > 0
        );
    }

}
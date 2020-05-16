

#include <GL/glew.h>

#include "Chunk.h"
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
        Chunk* lastX = nullptr;
        Chunk* lastY = nullptr;
        Chunk* lastZ = nullptr;
        for (auto i = 0; i < viewDistanceInChunks.x * 2; i++)
        {
            for (auto j = 0; j < viewDistanceInChunks.y * 2; j++)
            {
                for (auto k = 0; k < viewDistanceInChunks.z * 2; k++)
                {
                    auto worldSize = convertViewDistance(glm::ivec3{
                        i - viewDistanceInChunks.x,
                        j - viewDistanceInChunks.y,
                        k - viewDistanceInChunks.z
                    });
                    auto chunk = std::make_unique<Chunk>(worldSize, texture, BlockType::DIRT);
                    
                    chunk->setNeighbor(lastX, Direction::NegX);
                    if (lastX) {
                        lastX->setNeighbor(chunk.get(), Direction::PlusX);
                    }
                    chunk->setNeighbor(lastY, Direction::NegY);
                    if (lastY) {
                        lastY->setNeighbor(chunk.get(), Direction::PlusY);
                    }
                    chunk->setNeighbor(lastZ, Direction::NegZ);
                    if (lastZ) {
                        lastZ->setNeighbor(chunk.get(), Direction::PlusZ);
                    }

                    chunks.push_back(std::move(chunk));
                    lastZ = chunks[chunks.size() - 1].get();
                    if (j > 0) {
                        lastY = chunks[chunks.size() - viewDistanceInChunks.z * 2].get();
                    }
                    if (i > 0) {
                        lastX = chunks[chunks.size() - viewDistanceInChunks.z * 2 * viewDistanceInChunks.y * 2].get();
                    }
                }
                lastZ = nullptr;
                
            }
            lastY = nullptr;
            
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
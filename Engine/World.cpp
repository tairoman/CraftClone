

#include <GL/glew.h>
#include <glm/ext/vector_int3.hpp>
#include <glm/gtx/hash.hpp>

#include "Chunk.h"
#include "World.h"

#include "../lib/PerlinNoise.hpp"

#include <iostream>

namespace {

auto ivec3ToVec3(const glm::ivec3& ivec)
{
    return glm::vec3{ ivec.x, ivec.y, ivec.z };
}

auto convertViewDistance(glm::ivec3 viewDistanceInChunks)
{
    // Converts view distance in the form of number of chunks in every direction to actual world length
    return glm::vec3 {
        viewDistanceInChunks.x * ChunkData::BLOCKS_X * ChunkData::BLOCK_WORLD_EXTENT,
        viewDistanceInChunks.y * ChunkData::BLOCKS_Y * ChunkData::BLOCK_WORLD_EXTENT,
        viewDistanceInChunks.z * ChunkData::BLOCKS_Z * ChunkData::BLOCK_WORLD_EXTENT
    };
}

} // anon namespace

namespace Engine
{

World::World(glm::ivec3 viewDistanceInChunks, GLuint texture)
    : viewDistance(convertViewDistance(viewDistanceInChunks))
{
    siv::BasicPerlinNoise<float> perlinNoise;
    chunks.reserve(viewDistanceInChunks.x * viewDistanceInChunks.y * viewDistanceInChunks.z);
    Chunk* lastX = nullptr;
    Chunk* lastY = nullptr;
    Chunk* lastZ = nullptr;
    for (auto i = 0; i < viewDistanceInChunks.x * 2; i++) {
        for (auto j = 0; j < viewDistanceInChunks.y; j++) {
            for (auto k = 0; k < viewDistanceInChunks.z * 2; k++) {
                const auto worldPos = convertViewDistance({
                    i - viewDistanceInChunks.x,
                    j - viewDistanceInChunks.y,
                    k - viewDistanceInChunks.z
                });
                auto chunk = std::make_unique<Chunk>(worldPos, texture, BlockType::DIRT);
                for (auto a = 0; a < ChunkData::BLOCKS_X; a++) {
                    for (auto b = 0; b < ChunkData::BLOCKS_Z; b++) {
                        const auto fx = 64;
                        const auto fz = 64;
                        auto value = int(std::floor(std::pow(perlinNoise.accumulatedOctaveNoise2D_0_1((worldPos.x + a) / fx, (worldPos.z + b) / fz, 4), 7) * ChunkData::BLOCKS_Y));
                        //std::cout << "value: " << value << "\n";
                        while (value > 0) {
                            chunk->set(a, ChunkData::BLOCKS_Y - value, b, BlockType::AIR);
                            value--;
                        }
                    }
                }
                
                const auto centerPos = chunk->getCenterPos();

                if (k > 0) {
                    auto pos = centerPos + glm::ivec3(0,0,-ChunkData::BLOCKS_Z);
                    auto hash = std::hash<glm::ivec3>{}(pos);
                    auto it = chunks.find(hash);
                    lastZ = it == chunks.end() ? nullptr : (*it).second.get();
                }
                if (j > 0) {
                    auto pos = centerPos + glm::ivec3(0,-ChunkData::BLOCKS_Y,0);
                    auto hash = std::hash<glm::ivec3>{}(pos);
                    auto it = chunks.find(hash);
                    lastY = it == chunks.end() ? nullptr : (*it).second.get();
                }
                if (i > 0) {
                    const auto pos = centerPos + glm::ivec3(-ChunkData::BLOCKS_X,0,0);
                    const auto hash = std::hash<glm::ivec3>{}(pos);
                    auto it = chunks.find(hash);
                    lastX = it == chunks.end() ? nullptr : (*it).second.get();
                }

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

                const auto hashed = std::hash<glm::ivec3>{}(centerPos);
                
                chunks[hashed] = std::move(chunk);
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
    for (const auto& [_, chunk] : chunks)
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
    const auto chunkPos = ivec3ToVec3(chunk->getCenterPos()) - playerPos;
    return (
        std::abs(viewDistance.x - chunkPos.x) > 0 &&
        std::abs(viewDistance.y - chunkPos.y) > 0 &&
        std::abs(viewDistance.z - chunkPos.z) > 0
    );
}

}
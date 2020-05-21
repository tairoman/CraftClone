

#include <GL/glew.h>
#include <glm/ext/vector_int3.hpp>
#include <glm/gtx/hash.hpp>

#include "Chunk.h"
#include "World.h"

#include <iostream>

namespace {
    decltype(auto) ivec3ToVec3(const glm::ivec3& ivec)
    {
        return glm::vec3{ ivec.x, ivec.y, ivec.z };
    }
}

namespace Engine
{

static glm::vec3 convertViewDistance(glm::ivec3 viewDistanceInChunks)
{
    // Converts view distance in the form of number of chunks in every direction to actual world length
    return {
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
                auto worldPos = convertViewDistance({
                    i - viewDistanceInChunks.x,
                    j - viewDistanceInChunks.y,
                    k - viewDistanceInChunks.z
                });
                auto chunk = std::make_unique<Chunk>(worldPos, texture, BlockType::DIRT);
                
                const auto centerPos = chunk->getCenterPos();

                if (k > 0) {
                    auto pos = centerPos + glm::ivec3(0,0,-ChunkData::BLOCKS_Z);
                    auto hash = std::hash<glm::ivec3>{}(pos);
                    auto it = chunks.find(hash);
                    lastZ = it == chunks.end() ? nullptr : (*it).second.get();
                    // if (it == chunks.end()) {
                    //     std::cout << "No entry with hash: " << hash << "\n\t Pos: (" << pos.x << "," << pos.y << ", " << pos.z << ")\n";
                    // }
                    // else {
                    //     std::cout << "Found entry with hash: " << hash << "\n\t Pos: (" << pos.x << "," << pos.y << ", " << pos.z << ")\n";
                    // }
                }
                if (j > 0) {
                    auto pos = centerPos + glm::ivec3(0,-ChunkData::BLOCKS_Y,0);
                    auto hash = std::hash<glm::ivec3>{}(pos);
                    auto it = chunks.find(hash);
                    lastY = it == chunks.end() ? nullptr : (*it).second.get();
                    // if (it == chunks.end()) {
                    //     std::cout << "No entry with hash: " << hash << "\n\t Pos: (" << pos.x << "," << pos.y << ", " << pos.z << ")\n";
                    // }
                    // else {
                    //     std::cout << "Found entry with hash: " << hash << "\n\t Pos: (" << pos.x << "," << pos.y << ", " << pos.z << ")\n";
                    // }
                }
                if (i > 0) {
                    auto pos = centerPos + glm::ivec3(-ChunkData::BLOCKS_X,0,0);
                    auto hash = std::hash<glm::ivec3>{}(pos);
                    auto it = chunks.find(hash);
                    lastX = it == chunks.end() ? nullptr : (*it).second.get();
                    // if (it == chunks.end()) {
                    //     std::cout << "No entry with hash: " << hash << "\n\t Pos: (" << pos.x << "," << pos.y << ", " << pos.z << ")\n";
                    // }
                    // else {
                    //     std::cout << "Found entry with hash: " << hash << "\n\t Pos: (" << pos.x << "," << pos.y << ", " << pos.z << ")\n";
                    // }
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
                //std::cout << "Added chunk with hash " << hashed << "\n\t Pos: (" << centerPos.x << "," << centerPos.y << ", " << centerPos.z << ")\n";

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
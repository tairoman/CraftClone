

#include <GL/glew.h>
#include <glm/ext/vector_int3.hpp>
#include <glm/gtx/hash.hpp>

#include "Chunk.h"
#include "World.h"

#include <iostream>

namespace
{

glm::vec3 ivec3ToVec3(const glm::ivec3& ivec)
{
    return { ivec.x, ivec.y, ivec.z };
}

glm::ivec3 chunkIndexToPos(glm::ivec3 viewDistanceInChunks)
{
    // Converts view distance in the form of number of chunks in every direction to actual world length
    return {
        viewDistanceInChunks.x * ChunkData::BLOCKS_X * ChunkData::BLOCK_WORLD_EXTENT,
        viewDistanceInChunks.y * ChunkData::BLOCKS_Y * ChunkData::BLOCK_WORLD_EXTENT,
        viewDistanceInChunks.z * ChunkData::BLOCKS_Z * ChunkData::BLOCK_WORLD_EXTENT
    };
}

glm::ivec3 posToChunkIndex(glm::ivec3 pos)
{
    return {
        int(std::floor(pos.x / ChunkData::BLOCKS_X)),
        int(std::floor(pos.y / ChunkData::BLOCKS_Y)),
        int(std::floor(pos.z / ChunkData::BLOCKS_Z))
    };
}

} // anon namespace

namespace Engine
{

World::World(glm::ivec3 viewDistanceInChunks, GLuint texture)
    : viewDistance(chunkIndexToPos(viewDistanceInChunks))
    , m_texture(texture)
{
    chunks.reserve(viewDistanceInChunks.x * viewDistanceInChunks.y * viewDistanceInChunks.z);
    for (auto i = 0; i < viewDistanceInChunks.x * 2; i++) {
        for (auto j = 0; j < viewDistanceInChunks.y; j++) {
            for (auto k = 0; k < viewDistanceInChunks.z * 2; k++) {
                const auto worldPos = glm::ivec3 {
                    i - viewDistanceInChunks.x,
                    j - viewDistanceInChunks.y,
                    k - viewDistanceInChunks.z
                };
                addChunkAt(worldPos, texture);
            }   
        }
    }
}

void World::set(int x, int y, int z, BlockType type)
{
    // Get the lower-left corner (start) position of the chunk
    const auto chunkPos = posToChunkIndex({
        int(std::floor(x / ChunkData::BLOCKS_X)),
        int(std::floor(y / ChunkData::BLOCKS_Y)),
        int(std::floor(z / ChunkData::BLOCKS_Z))
    });
    
    auto chunk = chunkAt(chunkPos);
    if (!chunk) {
        chunk = addChunkAt(chunkPos, m_texture);
    }

    chunk->set(x % ChunkData::BLOCKS_X, y % ChunkData::BLOCKS_Y, z % ChunkData::BLOCKS_Z, type);
}

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

Chunk* World::chunkAt(const glm::ivec3& pos) const
{
    auto hash = std::hash<glm::ivec3>{}(pos);
    auto it = chunks.find(hash);
    return it == chunks.end() ? nullptr : (*it).second.get();
}

Chunk* World::addChunkAt(const glm::ivec3& pos, GLuint texture)
{
    const auto worldPos = chunkIndexToPos(pos);
    auto chunk = std::make_unique<Chunk>(worldPos, texture, BlockType::DIRT);
    for (auto a = 0; a < ChunkData::BLOCKS_X; a++) {
        for (auto b = 0; b < ChunkData::BLOCKS_Z; b++) {
            const auto fx = 256.0f;
            const auto fz = 256.0f;
            auto value = int(std::floor(std::pow(m_perlinNoise.accumulatedOctaveNoise2D_0_1(float(worldPos.x + a) / fx, float(worldPos.z + b) / fz, 5), 2) * ChunkData::BLOCKS_Y));
            for (auto i = ChunkData::BLOCKS_Y-1; i >= value; i--) {
                chunk->set(a, i, b, BlockType::AIR);
            }
        }
    }

    auto lastZ = chunkAt(pos + glm::ivec3(0,0,-1));

    auto lastY = chunkAt(pos + glm::ivec3(0,-1,0));

    auto lastX = chunkAt(pos + glm::ivec3(-1,0,0));

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

    const auto hashed = std::hash<glm::ivec3>{}(pos);

    const auto observer = chunk.get();
    
    chunks[hashed] = std::move(chunk);

    return observer;
}

}


#include <GL/glew.h>
#include <glm/ext/vector_int3.hpp>
#include <glm/gtx/hash.hpp>

#include "Chunk.h"
#include "World.h"

#include <iostream>
#include <optional>


namespace Engine
{

World::World(glm::ivec3 viewDistanceInChunks, GLuint texture)
    : viewDistance(viewDistanceInChunks)
    , m_texture(texture)
{
    chunks.reserve(viewDistanceInChunks.x * viewDistanceInChunks.y * viewDistanceInChunks.z);

    m_chunkGeneratorThread = std::thread([this](){

        std::cout << "Starting chunk generator thread\n";

        while (!m_stopChunkGeneratorThread.load()) {
            glm::ivec3 playerChunk;
            {
                std::unique_lock<std::mutex> lck(m_playerChunkMutex);
                m_newPlayerChunkIndex.wait(lck, [this](){
                    return m_playerChunk.has_value() || m_stopChunkGeneratorThread.load();
                });
                
                if (m_stopChunkGeneratorThread.load()) {
                    return;
                }

                // Now m_playerChunk must have a value.
                // Let's copy it to allow other thread to change m_playerChunk
                // until next time we execute here.
                playerChunk = *m_playerChunk;
                m_playerChunk = {};
            }

            for (auto x = -viewDistance.x; x < viewDistance.x; x++) {
                for (auto y = -viewDistance.y; y < viewDistance.y; y++) {
                    for (auto z = -viewDistance.z; z < viewDistance.z; z++) {
                        if (m_stopChunkGeneratorThread.load()) {
                            return;
                        }
                        ensureChunkAtIndex({playerChunk.x + x, playerChunk.y + y, playerChunk.z + z});
                    }
                }
            }
        }

        std::cout << "Ending chunk generator thread\n";
    });
}

World::~World()
{
    m_stopChunkGeneratorThread = true;
    m_newPlayerChunkIndex.notify_all();
    m_chunkGeneratorThread.join();
}

glm::ivec3 World::chunkIndexToPos(glm::ivec3 viewDistanceInChunks)
{
    // Converts view distance in the form of number of chunks in every direction to actual world length
    return {
        viewDistanceInChunks.x * ChunkData::BLOCKS_X * ChunkData::BLOCK_WORLD_EXTENT,
        viewDistanceInChunks.y * ChunkData::BLOCKS_Y * ChunkData::BLOCK_WORLD_EXTENT,
        viewDistanceInChunks.z * ChunkData::BLOCKS_Z * ChunkData::BLOCK_WORLD_EXTENT
    };
}

glm::ivec3 World::posToChunkIndex(glm::ivec3 pos)
{
    return {
        int(std::floor(pos.x / ChunkData::BLOCKS_X)),
        int(std::floor(pos.y / ChunkData::BLOCKS_Y)),
        int(std::floor(pos.z / ChunkData::BLOCKS_Z))
    };
}

void World::set(int x, int y, int z, BlockType type)
{
    // Get the lower-left corner (start) position of the chunk
    const auto chunkPos = posToChunkIndex({
        int(std::floor(x / ChunkData::BLOCKS_X)),
        int(std::floor(y / ChunkData::BLOCKS_Y)),
        int(std::floor(z / ChunkData::BLOCKS_Z))
    });
    
    auto chunk = ensureChunkAtIndex(chunkPos);

    chunk->set(x % ChunkData::BLOCKS_X, y % ChunkData::BLOCKS_Y, z % ChunkData::BLOCKS_Z, type);
}

void World::render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix)
{
    renderChunks(playerPos, shader, viewProjectionMatrix);
}

void World::renderChunks(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix)
{
    std::unique_lock<std::mutex> lck(m_chunksMutex);
    std::vector<std::size_t> outsideChunkKeys;
    for (auto& [key, chunk] : chunks)
    {
        if (isWithinViewDistance(chunk.get(), playerPos))
        {
            shader.setUniform("modelViewProjectionMatrix", viewProjectionMatrix * chunk->getModelWorldMatrix());
            chunk->render();
        }
        else
        {
            // Don't render and tag for replacement
            outsideChunkKeys.push_back(key);
        }
    }

    for (auto key : outsideChunkKeys) {
        chunks.erase(key);
    }
}

bool World::isWithinViewDistance(Chunk* chunk, const glm::vec3& playerPos) const
{
    // Get chunk position relative to player

    const auto chunkDiff = posToChunkIndex(chunk->pos()) - posToChunkIndex(playerPos);

    return (
        viewDistance.x >= std::abs(chunkDiff.x) &&
        viewDistance.y >= std::abs(chunkDiff.y) &&
        viewDistance.z >= std::abs(chunkDiff.z)
    );
}

Chunk* World::chunkAt(const glm::ivec3& pos) const
{
    std::unique_lock<std::mutex> lck(m_chunksMutex);
    auto hash = std::hash<glm::ivec3>{}(pos);
    auto it = chunks.find(hash);
    return it == chunks.end() ? nullptr : (*it).second.get();
}

Chunk* World::ensureChunkAtIndex(const glm::ivec3& chunkIndex)
{
    auto chunk = chunkAt(chunkIndex);
    if (!chunk) {
        chunk = addChunkAt(chunkIndex, m_texture);
    }
    return chunk;
}

Chunk* World::addChunkAt(const glm::ivec3& chunkIndex, GLuint texture)
{
    const auto worldPos = chunkIndexToPos(chunkIndex);
    auto chunk = std::make_unique<Chunk>(worldPos, texture, BlockType::AIR);
    for (auto a = 0; a < ChunkData::BLOCKS_X; a++) {
        for (auto b = 0; b < ChunkData::BLOCKS_Z; b++) {
            const auto heightFreq = 256.0f;
            const auto densityFreq = 64.0f;
            const auto maxHeight = 200;
            auto heightValue = int(std::floor(std::pow(m_perlinNoise.accumulatedOctaveNoise2D_0_1(float(worldPos.x + a) / heightFreq, float(worldPos.z + b) / heightFreq, 5), 2) * maxHeight));
            auto firstBlock = std::optional<int>();
            auto startPos = heightValue >= (worldPos.y + ChunkData::BLOCKS_Y) ? ChunkData::BLOCKS_Y - 1 : heightValue - worldPos.y;
            for (auto c = startPos; c >= 0; c--) {
                auto value =  0.8 * m_perlinNoise.accumulatedOctaveNoise3D_0_1(float(worldPos.x + a) / densityFreq, float(worldPos.y + c) / densityFreq, float(worldPos.z + b) / densityFreq, 3);
                const auto addBlock = value < 0.7f;
                if (!addBlock) {
                    continue;
                }

                auto blockType = BlockType::STONE;
                if (!firstBlock.has_value()) {
                    firstBlock = c;
                    blockType = BlockType::GRASS;
                }
                else if (c > (*firstBlock - 3)) {
                    blockType = BlockType::DIRT;
                }
                chunk->set(a, c, b, blockType);
            }
        }
    }

    auto lastZ = chunkAt(chunkIndex + glm::ivec3(0,0,-1));

    auto lastY = chunkAt(chunkIndex + glm::ivec3(0,-1,0));

    auto lastX = chunkAt(chunkIndex + glm::ivec3(-1,0,0));

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

    chunk->regenerateMesh();

    const auto hashed = std::hash<glm::ivec3>{}(chunkIndex);

    const auto observer = chunk.get();

    {
        std::unique_lock<std::mutex> lck(m_chunksMutex);
        chunks[hashed] = std::move(chunk);
    }

    return observer;
}

void World::setPlayerChunk(glm::ivec3 chunkIndex)
{
    std::unique_lock<std::mutex> lck(m_playerChunkMutex);
    m_playerChunk = chunkIndex;
    m_newPlayerChunkIndex.notify_one();
}

}
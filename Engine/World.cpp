

#include <GL/glew.h>
#include <glm/ext/vector_int3.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/component_wise.hpp>

#include "Chunk.h"
#include "World.h"
#include "utils/Chunkindex.h"

#include <iostream>
#include <optional>

namespace
{
    const auto viewDistanceInChunks = chunkViewDistance();
}

namespace Engine
{

World::World(GLuint texture)
    : m_texture(texture)
{
    chunks.reserve(viewDistanceInChunks.x * viewDistanceInChunks.y * viewDistanceInChunks.z);

    m_chunkGeneratorThread = std::thread([this](){

        const auto nextIndex = [this](const ChunkIndex& lastOffset) -> std::optional<ChunkIndex> {

            const auto& offsetData = glm::ivec3{ lastOffset.data().x, 0, lastOffset.data().z };

            const auto currentRing = glm::compMax(glm::abs(offsetData));

            if (!isWithinViewDistance(lastOffset.data())) {
                return std::nullopt;
            }

            if (currentRing == 0) {
                return ChunkIndex{ glm::ivec3{-1, 0, -1} };
            }

            if (offsetData.z == -currentRing) {
                if (offsetData.x < currentRing) {
                    return ChunkIndex{ glm::ivec3{ offsetData.x + 1, lastOffset.data().y, offsetData.z} };
                }
            }

            if (offsetData.x == currentRing) {
                if (offsetData.z < currentRing) {
                    return ChunkIndex{ glm::ivec3{ offsetData.x, lastOffset.data().y, offsetData.z + 1 } };
                }
            }

            if (offsetData.z == currentRing) {
                if (offsetData.x > -currentRing) {
                    return ChunkIndex{ glm::ivec3{ offsetData.x - 1, lastOffset.data().y, offsetData.z } };
                }
            }

            if (offsetData.x == -currentRing) {
                // We don't want to return {-currentRing, -currentRing} because that signals we want to change ring
                if (offsetData.z > -currentRing + 1) {
                    return ChunkIndex{ glm::ivec3{ offsetData.x, lastOffset.data().y, offsetData.z - 1 } };
                }
            }

            // Go to next ring
            return ChunkIndex{ glm::ivec3{ offsetData.x - 1, lastOffset.data().y, offsetData.z - 2 } };
        };

        std::cout << "Starting chunk generator thread with id: " << std::this_thread::get_id() << "\n";

        while (!m_stopChunkGeneratorThread.load()) {

            // Will block until new event on queue
            auto event = m_eventQueue.nextEvent();

            if (auto generateChunkEvent = dynamic_cast<GenerateChunkEvent*>(event.get())) {
                // No value should only be the case before first playerchunk has been set
                // and therefore before first NewOriginChunkEvent.
                assert(m_playerChunk.has_value());
                const auto index = glm::ivec3{ generateChunkEvent->origin().data() + generateChunkEvent->offset().data() };
                if (generateChunkEvent->origin().data() != m_playerChunk.value().data()) {
                    // TODO: Check if within range and then maybe generate it anyway instead of having the same event later?
                    continue; // Origin has changed, ignore this chunk for now
                }
               
                auto nextIndexOpt = nextIndex(generateChunkEvent->offset());
                if (!nextIndexOpt.has_value()) {
                    // Outside of view distance -> generate no new chunks
                    continue;
                }

                for (auto yIndex = viewDistanceInChunks.y; yIndex > -viewDistanceInChunks.y; yIndex--) {
                    ensureChunkAtIndex(ChunkIndex{ index + glm::ivec3{0, yIndex, 0} });
                }

                m_eventQueue.addEvent(std::make_unique<GenerateChunkEvent>(m_playerChunk.value(), nextIndexOpt.value()));
            }
            else if (auto newOriginChunkEvent = dynamic_cast<NewOriginChunkEvent*>(event.get())) {
                m_playerChunk = newOriginChunkEvent->index();
                std::vector<std::size_t> outsideChunkKeys;
                {
                    std::unique_lock<std::mutex> lck(m_chunksMutex);
                    for (auto& [key, chunk] : chunks) {
                        if (!isWithinViewDistance(chunk.get(), newOriginChunkEvent->index().toWorldPos())) {
                            outsideChunkKeys.push_back(key);
                        }
                    }
                    for (auto key : outsideChunkKeys) {
                        chunks.erase(key);
                    }
                }
                // New start for chunk generation
                m_eventQueue.addEvent(std::make_unique<GenerateChunkEvent>(m_playerChunk.value(), ChunkIndex{ {0,0,0} }));
            }
        }

        std::cout << "Ending chunk generator thread\n";
    });
}

World::~World()
{
    m_stopChunkGeneratorThread = true;
    m_chunkGeneratorThread.join();
}

void World::set(int x, int y, int z, BlockType type)
{
    assert(false);
    //TODO: Implement with event
    //chunk->set(x % ChunkData::BLOCKS_X, y % ChunkData::BLOCKS_Y, z % ChunkData::BLOCKS_Z, type);
}

void World::render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix)
{
    renderChunks(playerPos, shader, viewProjectionMatrix);
}

void World::renderChunks(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix)
{
        std::unique_lock<std::mutex> lck(m_chunksMutex);
        for (auto& [key, chunk] : chunks)
        {
            shader.setUniform("modelViewProjectionMatrix", viewProjectionMatrix * chunk->getModelWorldMatrix());
            chunk->render();
        }
}

bool World::isWithinViewDistance(Chunk* chunk, const glm::vec3& playerPos) const
{
    // Get chunk position relative to player

    return isWithinViewDistance(ChunkIndex::fromWorldPos(chunk->pos()), ChunkIndex::fromWorldPos(playerPos));
}

bool World::isWithinViewDistance(const ChunkIndex& chunk, const ChunkIndex& playerChunk) const
{
    const auto chunkDiff = chunk.data() - playerChunk.data();

    return isWithinViewDistance(chunkDiff);
}

bool World::isWithinViewDistance(const glm::ivec3& offset) const
{
    return (
        viewDistanceInChunks.x >= std::abs(offset.x) &&
        viewDistanceInChunks.y >= std::abs(offset.y) &&
        viewDistanceInChunks.z >= std::abs(offset.z)
        );
}

Chunk* World::chunkAt(const ChunkIndex& index) const
{
    std::unique_lock<std::mutex> lck(m_chunksMutex);
    auto hash = std::hash<glm::ivec3>{}(index.data());
    auto it = chunks.find(hash);
    return it == chunks.end() ? nullptr : (*it).second.get();
}

void World::ensureChunkAtIndex(const ChunkIndex& index)
{
    auto chunk = chunkAt(index);
    if (!chunk) {
        addChunkAt(index, m_texture);
    }
}

void World::addChunkAt(const ChunkIndex& index, GLuint texture)
{
    const auto worldPos = index.toWorldPos();
    auto chunk = std::make_unique<Chunk>(worldPos, texture, BlockType::AIR);
    const auto chunkAbove = chunkAt(ChunkIndex{ index.data() + glm::ivec3{0,1,0} });
    for (auto a = 0; a < ChunkData::BLOCKS_X; a++) {
        for (auto b = 0; b < ChunkData::BLOCKS_Z; b++) {
            const auto heightFreq = 256.0f;
            const auto densityFreq = 32.0f;
            const auto maxHeight = 200;
            auto heightValue = int(std::floor(std::pow(m_perlinNoise.accumulatedOctaveNoise2D_0_1(float(worldPos.x + a) / heightFreq, float(worldPos.z + b) / heightFreq, 5), 2) * maxHeight));
            auto startPos = heightValue >= (worldPos.y + ChunkData::BLOCKS_Y) ? ChunkData::BLOCKS_Y - 1 : heightValue - worldPos.y;
            auto firstBlock = chunkAbove ? (chunkAbove->get(a, 0, b) == BlockType::AIR ? std::nullopt : std::make_optional(std::numeric_limits<int>::max())) : std::nullopt;
            for (auto c = startPos; c >= 0; c--) {
                auto value = m_perlinNoise.accumulatedOctaveNoise3D_0_1(float(worldPos.x + a) / densityFreq, float(worldPos.y + c) / densityFreq, float(worldPos.z + b) / densityFreq, 2);
                const auto addBlock = value < 0.7f;
                if (!addBlock) {
                    continue;
                }

                auto blockType = BlockType::STONE;
                if (!firstBlock.has_value() && (worldPos.y + c) >= 0) {
                    firstBlock = c;
                    blockType = BlockType::GRASS;
                }
                else if (firstBlock.has_value() && c > (*firstBlock - 3)) {
                    blockType = BlockType::DIRT;
                }
                chunk->set(a, c, b, blockType);
            }
        }
    }

    auto lastZ = chunkAt(ChunkIndex{ index.data() + glm::ivec3(0,0,-1) });

    auto lastY = chunkAt(ChunkIndex{ index.data() + glm::ivec3(0,-1,0) });

    auto lastX = chunkAt(ChunkIndex{ index.data() + glm::ivec3(-1,0,0) });

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

    const auto hashed = std::hash<glm::ivec3>{}(index.data());

    {
        std::unique_lock<std::mutex> lck(m_chunksMutex);
        chunks[hashed] = std::move(chunk);
    }
}

void World::setPlayerChunk(ChunkIndex index)
{
    m_eventQueue.addEvent(std::make_unique<NewOriginChunkEvent>(index));
}

}
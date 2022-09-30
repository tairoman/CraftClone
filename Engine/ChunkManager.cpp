#include "ChunkManager.h"
#include "Chunk.h"
#include "Shader.h"
#include "utils/Chunkindex.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/component_wise.hpp>

#include <iostream>
#include <functional>

namespace
{
    const auto viewDistanceInChunks = chunkViewDistance();
}

class RemoveChunksEvent : public Event
{
public:
    RemoveChunksEvent(std::vector<std::size_t> chunkHashes) : Event(100), m_chunkHashes(std::move(chunkHashes)) {}
    ~RemoveChunksEvent() override = default;

    [[nodiscard]] const std::vector<std::size_t>& chunkHashes() const { return m_chunkHashes; };

private:
    std::vector<std::size_t> m_chunkHashes;
};

class GenerateChunkEvent : public Event
{
public:
    GenerateChunkEvent(ChunkIndex originIndex, ChunkIndex offsetIndex) :
        Event(10),
        m_originIndex(std::move(originIndex)),
        m_offsetIndex(std::move(offsetIndex))
    {}

    ~GenerateChunkEvent() override = default;

    [[nodiscard]] const ChunkIndex& origin() const { return m_originIndex; }
    [[nodiscard]] const ChunkIndex& offset() const { return m_offsetIndex; }

private:
    ChunkIndex m_originIndex;
    ChunkIndex m_offsetIndex;
};

class NewOriginChunkEvent : public Event
{
public:
    NewOriginChunkEvent(ChunkIndex index) : Event(1), m_index(std::move(index)) {}
    ~NewOriginChunkEvent() override = default;

    [[nodiscard]] const ChunkIndex& index() const { return m_index; }

private:
    ChunkIndex m_index;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class ChunkManagerThread : public EventThread
{
public:
    ChunkManagerThread(ChunkManager* parent);
    ~ChunkManagerThread() override = default;

    void onStart() override;
    void handleEvent(Event* ev) override;

    ChunkManager* const m_parent = nullptr;
};

ChunkManagerThread::ChunkManagerThread(ChunkManager* parent)
    : m_parent(parent)
{
}

void ChunkManagerThread::onStart()
{
    std::cout << "Starting chunk generator thread with id: " << std::this_thread::get_id() << "\n";
}

void ChunkManagerThread::handleEvent(Event* ev)
{
    if (auto generateChunkEvent = dynamic_cast<GenerateChunkEvent*>(ev)) {
        // No value should only be the case before first playerchunk has been set
        // and therefore before first NewOriginChunkEvent.
        assert(m_parent->m_playerChunk.has_value());
        const auto index = glm::ivec3{ generateChunkEvent->origin().data() + generateChunkEvent->offset().data() };
        if (generateChunkEvent->origin().data() != m_parent->m_playerChunk.value().data()) {
            // TODO: Check if within range and then maybe generate it anyway instead of having the same event later?
            return; // Origin has changed, ignore this chunk for now
        }

        const auto nextIndex = [this](const ChunkIndex& lastOffset) -> std::optional<ChunkIndex> {

            const auto& offsetData = glm::ivec3{ lastOffset.data().x, 0, lastOffset.data().z };

            const auto currentRing = glm::compMax(glm::abs(offsetData));

            if (!m_parent->isWithinViewDistance(lastOffset.data())) {
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

        auto nextIndexOpt = nextIndex(generateChunkEvent->offset());
        if (!nextIndexOpt.has_value()) {
            // Outside of view distance -> generate no new chunks
            return;
        }

        for (auto yIndex = viewDistanceInChunks.y; yIndex > -viewDistanceInChunks.y; yIndex--) {
            m_parent->ensureChunkAtIndex(ChunkIndex{ index + glm::ivec3{0, yIndex, 0} });
        }

        pushEvent(std::make_unique<GenerateChunkEvent>(m_parent->m_playerChunk.value(), nextIndexOpt.value()));
    }
    else if (auto newOriginChunkEvent = dynamic_cast<NewOriginChunkEvent*>(ev)) {
        m_parent->m_playerChunk = newOriginChunkEvent->index();
        std::vector<std::size_t> outsideChunkKeys;
        {
            std::unique_lock<std::mutex> lck(m_parent->m_chunksMutex);
            for (auto& [key, chunk] : m_parent->m_chunks) {
                if (!m_parent->isWithinViewDistance(chunk.get(), newOriginChunkEvent->index().toWorldPos())) {
                    outsideChunkKeys.push_back(key);
                }
            }
            for (auto key : outsideChunkKeys) {
                m_parent->m_chunks.erase(key);
            }
        }
        // New start for chunk generation
        pushEvent(std::make_unique<GenerateChunkEvent>(m_parent->m_playerChunk.value(), ChunkIndex{ {0,0,0} }));
    }
    else {
        std::cout << "Event not handled by ChunkGeneratorThread: " << typeid(ev).name() << "\n";
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

ChunkManager::ChunkManager(GLuint texture)
    : m_thread(new ChunkManagerThread{ this })
{
    m_chunks.reserve(viewDistanceInChunks.x * viewDistanceInChunks.y * viewDistanceInChunks.z);

    sourceChunk.onChange.listen(m_observer, [this](const glm::ivec3& index) {
        m_thread->pushEvent(std::make_unique<NewOriginChunkEvent>(ChunkIndex{ index }));
    });
}

ChunkManager::~ChunkManager()
{
    m_thread->stop();
    m_thread->join();
}

bool ChunkManager::isWithinViewDistance(Engine::Chunk* chunk, const glm::vec3& playerPos) const
{
    // Get chunk position relative to player

    return isWithinViewDistance(ChunkIndex::fromWorldPos(chunk->pos()), ChunkIndex::fromWorldPos(playerPos));
}

bool ChunkManager::isWithinViewDistance(const ChunkIndex& chunk, const ChunkIndex& playerChunk) const
{
    const auto chunkDiff = chunk.data() - playerChunk.data();

    return isWithinViewDistance(chunkDiff);
}

bool ChunkManager::isWithinViewDistance(const glm::ivec3& offset) const
{
    return (
        viewDistanceInChunks.x >= std::abs(offset.x) &&
        viewDistanceInChunks.y >= std::abs(offset.y) &&
        viewDistanceInChunks.z >= std::abs(offset.z)
        );
}

Engine::Chunk* ChunkManager::chunkAt(const ChunkIndex& index) const
{
    std::unique_lock<std::mutex> lck(m_chunksMutex);
    auto hash = std::hash<glm::ivec3>{}(index.data());
    auto it = m_chunks.find(hash);
    return it == m_chunks.end() ? nullptr : (*it).second.get();
}

void ChunkManager::renderChunks(const glm::vec3& playerPos, const Engine::Shader& shader, const glm::mat4& viewProjectionMatrix)
{
    std::unique_lock<std::mutex> lck(m_chunksMutex);
    for (auto& [key, chunk] : m_chunks)
    {
        shader.setUniform("modelViewProjectionMatrix", viewProjectionMatrix * chunk->getModelWorldMatrix());
        chunk->render();
    }
}

void ChunkManager::ensureChunkAtIndex(const ChunkIndex& index)
{
    auto chunk = chunkAt(index);
    if (!chunk) {
        addChunkAt(index, m_texture);
    }
}

void ChunkManager::addChunkAt(const ChunkIndex& index, GLuint texture)
{
    const auto worldPos = index.toWorldPos();
    auto chunk = std::make_unique<Engine::Chunk>(worldPos, texture, Engine::BlockType::AIR);
    const auto chunkAbove = chunkAt(ChunkIndex{ index.data() + glm::ivec3{0,1,0} });
    for (auto a = 0; a < ChunkData::BLOCKS_X; a++) {
        for (auto b = 0; b < ChunkData::BLOCKS_Z; b++) {
            const auto heightFreq = 256.0f;
            const auto densityFreq = 32.0f;
            const auto maxHeight = 200;
            auto heightValue = int(std::floor(std::pow(m_perlinNoise.accumulatedOctaveNoise2D_0_1(float(worldPos.x + a) / heightFreq, float(worldPos.z + b) / heightFreq, 5), 2) * maxHeight));
            auto startPos = heightValue >= (worldPos.y + ChunkData::BLOCKS_Y) ? ChunkData::BLOCKS_Y - 1 : heightValue - worldPos.y;
            auto firstBlock = chunkAbove ? (chunkAbove->get(a, 0, b) == Engine::BlockType::AIR ? std::nullopt : std::make_optional(std::numeric_limits<int>::max())) : std::nullopt;
            for (auto c = startPos; c >= 0; c--) {
                auto value = m_perlinNoise.accumulatedOctaveNoise3D_0_1(float(worldPos.x + a) / densityFreq, float(worldPos.y + c) / densityFreq, float(worldPos.z + b) / densityFreq, 2);
                const auto addBlock = value < 0.7f;
                if (!addBlock) {
                    continue;
                }

                auto blockType = Engine::BlockType::STONE;
                if (!firstBlock.has_value() && (worldPos.y + c) >= 0) {
                    firstBlock = c;
                    blockType = Engine::BlockType::GRASS;
                }
                else if (firstBlock.has_value() && c > (*firstBlock - 3)) {
                    blockType = Engine::BlockType::DIRT;
                }
                chunk->set(a, c, b, blockType);
            }
        }
    }

    auto lastZ = chunkAt(ChunkIndex{ index.data() + glm::ivec3(0,0,-1) });

    auto lastY = chunkAt(ChunkIndex{ index.data() + glm::ivec3(0,-1,0) });

    auto lastX = chunkAt(ChunkIndex{ index.data() + glm::ivec3(-1,0,0) });

    chunk->setNeighbor(lastX, Engine::Direction::NegX);
    if (lastX) {
        lastX->setNeighbor(chunk.get(), Engine::Direction::PlusX);
    }
    chunk->setNeighbor(lastY, Engine::Direction::NegY);
    if (lastY) {
        lastY->setNeighbor(chunk.get(), Engine::Direction::PlusY);
    }
    chunk->setNeighbor(lastZ, Engine::Direction::NegZ);
    if (lastZ) {
        lastZ->setNeighbor(chunk.get(), Engine::Direction::PlusZ);
    }

    chunk->regenerateMesh();

    const auto hashed = std::hash<glm::ivec3>{}(index.data());

    {
        std::unique_lock<std::mutex> lck(m_chunksMutex);
        m_chunks[hashed] = std::move(chunk);
    }
}

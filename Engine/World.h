
#ifndef CRAFTBONE_WORLD_H
#define CRAFTBONE_WORLD_H

#include <glm/fwd.hpp>
#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>

#include "Chunk.h"
#include "Shader.h"
#include "utils/Chunkindex.h"

#include "../lib/PerlinNoise.hpp"

namespace Engine
{

template <typename T>
struct SharedPtrComparator {
    bool operator()(const std::shared_ptr<T>& lhs,
        const std::shared_ptr<T>& rhs) const
    {
        return lhs < rhs;
    }
};

class Event
{
public:
    Event() = default;
    Event(std::size_t priority) : m_priority(priority) {}
    virtual ~Event() {};

    std::size_t priority() const { return m_priority;  }

    bool operator<(const Event& b) const
    {
        return m_priority < b.priority();
    }
private:
    std::size_t m_priority = 0;
};

class RemoveChunksEvent : public Event
{
public:
    RemoveChunksEvent(std::vector<std::size_t> chunkHashes) : m_chunkHashes(std::move(chunkHashes)) {}
    ~RemoveChunksEvent() override = default;

    const std::vector<std::size_t> chunkHashes() const { return m_chunkHashes; };

private:
    std::vector<std::size_t> m_chunkHashes;
};

class World
{
public:
    World(GLuint texture);
    ~World();

    void render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix);
    void set(int x, int y, int z, BlockType type);

    Chunk* ensureChunkAtIndex(const ChunkIndex& index);

    void setPlayerChunk(ChunkIndex index);

private:
    void renderChunks(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix);
    bool isWithinViewDistance(Chunk* chunk, const glm::vec3& playerPos) const;
    
    Chunk* addChunkAt(const ChunkIndex& index, GLuint texture);
    Chunk* chunkAt(const ChunkIndex& index) const;

    std::unordered_map<std::size_t, std::unique_ptr<Chunk>> chunks;
    siv::BasicPerlinNoise<float> m_perlinNoise;

    GLuint m_texture;

    std::thread m_chunkGeneratorThread;
    std::atomic<bool> m_stopChunkGeneratorThread = false;

    mutable std::mutex m_chunksMutex;

    std::optional<ChunkIndex> m_playerChunk = {};
    mutable std::mutex m_playerChunkMutex;
    std::condition_variable m_newPlayerChunkIndex;

    mutable std::mutex m_eventQueueMutex;
    std::condition_variable m_eventQueueCond;
    std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, SharedPtrComparator<Event>> m_events;
};

}


#endif //CRAFTBONE_WORLD_H

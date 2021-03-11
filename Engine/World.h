
#ifndef CRAFTBONE_WORLD_H
#define CRAFTBONE_WORLD_H

#include <glm/fwd.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>

#include "Chunk.h"
#include "Shader.h"

#include "../lib/PerlinNoise.hpp"

namespace Engine
{

class World
{
public:
    World(glm::ivec3 viewDistanceInChunks, GLuint texture);
    ~World();

    void render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix);
    void set(int x, int y, int z, BlockType type);

    Chunk* ensureChunkAtIndex(const glm::ivec3& chunkIndex);

    static glm::ivec3 chunkIndexToPos(glm::ivec3 viewDistanceInChunks);
    static glm::ivec3 posToChunkIndex(glm::ivec3 pos);

    void setPlayerChunk(glm::ivec3 chunkIndex);

private:
    void renderChunks(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix);
    bool isWithinViewDistance(Chunk* chunk, const glm::vec3& playerPos) const;
    
    Chunk* addChunkAt(const glm::ivec3& chunkIndex, GLuint texture);
    Chunk* chunkAt(const glm::ivec3& pos) const;

    std::unordered_map<std::size_t, std::unique_ptr<Chunk>> chunks;
    glm::ivec3 viewDistance;
    siv::BasicPerlinNoise<float> m_perlinNoise;

    GLuint m_texture;

    std::thread m_chunkGeneratorThread;
    std::atomic<bool> m_stopChunkGeneratorThread = false;

    mutable std::mutex m_chunksMutex;

    std::optional<glm::ivec3> m_playerChunk = {};
    mutable std::mutex m_playerChunkMutex;
    std::condition_variable m_newPlayerChunkIndex;
};

}


#endif //CRAFTBONE_WORLD_H

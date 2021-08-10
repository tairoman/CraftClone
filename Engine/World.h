#pragma once

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
#include "Camera.h"
#include "ChunkManager.h"
#include "events/Event.h"
#include "events/EventQueue.h"
#include "utils/Chunkindex.h"

#include "../lib/PerlinNoise.hpp"

namespace Engine
{

class World
{
public:
    World(GLuint texture, Camera* cam);
    ~World();

    void render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix);
    void set(int x, int y, int z, BlockType type);

private:

    GLuint m_texture;

    ChunkManager m_chunks;
};

}

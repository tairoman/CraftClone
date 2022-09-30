#pragma once

#include <glm/fwd.hpp>

#include "Camera.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "events/Event.h"
#include "events/EventQueue.h"
#include "utils/Chunkindex.h"
#include "utils/Observer.h"
#include "Shader.h"

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
    Observer m_observer;

    GLuint m_texture;

    ChunkManager m_chunks;
};

}



#include <GL/glew.h>
#include <glm/ext/vector_int3.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/component_wise.hpp>

#include "Chunk.h"
#include "World.h"
#include "utils/Chunkindex.h"

#include <iostream>
#include <optional>

namespace Engine
{

World::World(GLuint texture, Camera* cam)
    : m_texture(texture)
    , m_chunks(texture)
{
    m_chunks.sourceChunk.set(ChunkIndex::fromWorldPos(cam->position.get()).data());

    cam->position.onChange.listen([this](const glm::vec3& pos) {
        auto newChunkIndex = ChunkIndex::fromWorldPos(pos);
        m_chunks.sourceChunk.set(newChunkIndex.data());
    });
}

World::~World() = default;

void World::set(int x, int y, int z, BlockType type)
{
    assert(false);
    //TODO: Implement with event
    //chunk->set(x % ChunkData::BLOCKS_X, y % ChunkData::BLOCKS_Y, z % ChunkData::BLOCKS_Z, type);
}

void World::render(const glm::vec3& playerPos, const Shader& shader, const glm::mat4& viewProjectionMatrix)
{
    m_chunks.renderChunks(playerPos, shader, viewProjectionMatrix);
}



}
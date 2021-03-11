#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "Chunk.h"

namespace Engine
{

Chunk::Chunk(glm::vec3 pos, GLuint texture, BlockType typ)
    : m_modelWorldMatrix(glm::translate(glm::mat4{1.0f}, pos))
    , m_startPos(pos)
    , m_texture(texture)
{
    glGenBuffers(1, &m_vbo);
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, textureCoord)));
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    m_blocks.fill(typ);

    m_neighbors.fill(nullptr);
}

Chunk::~Chunk()
{
    using Pairs = std::vector<std::pair<Direction, Direction>>;
    static const auto s_dirPairs = Pairs {
        {Direction::NegX, Direction::PlusX},
        {Direction::PlusX, Direction::NegX},
        {Direction::NegY, Direction::PlusY},
        {Direction::PlusY, Direction::NegY},
        {Direction::NegZ, Direction::PlusZ},
        {Direction::PlusZ, Direction::NegZ},
    };

    // Remove references to this chunk
    for (auto& [neighborDir, thisDir] : s_dirPairs) {
        if (auto otherChunk = neighbor(neighborDir)) {
            otherChunk->setNeighbor(nullptr, thisDir);
        }
    }

    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

BlockType Chunk::get(int x, int y, int z) const
{
    return m_blocks.at(x + ChunkData::BLOCKS_X * (y + ChunkData::BLOCKS_Y * z));
}

void Chunk::set(int x, int y, int z, BlockType type)
{
    m_changed = true;
    m_blocks.at(x + ChunkData::BLOCKS_X * (y + ChunkData::BLOCKS_Y * z)) = type;
}

void Chunk::render()
{
    if (m_changed){
        updateVbo();
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glBindVertexArray(m_vao);

    glDrawArrays(GL_TRIANGLES, 0, m_vertices);
}

void Chunk::setNeighbor(Chunk* chunk, Direction dir)
{
    m_neighbors.at(std::size_t(dir)) = chunk;
}

Chunk* Chunk::neighbor(Direction dir)
{
    return m_neighbors.at(std::size_t(dir));
}

glm::ivec3 Chunk::pos() const
{
    return m_startPos;
}

glm::ivec3 Chunk::getCenterPos() const
{
    return {
        m_startPos.x + ChunkData::BLOCKS_X / 2,
        m_startPos.y + ChunkData::BLOCKS_Y / 2,
        m_startPos.z + ChunkData::BLOCKS_Z / 2
    };
}

void Chunk::updateVbo()
{
    m_changed = false;
    m_mesh.regenerate();

    m_vertices = m_mesh.vertices().size();

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_mesh.vertices().size() * sizeof(Vertex), m_mesh.vertices().data(), GL_STATIC_DRAW);

}
}

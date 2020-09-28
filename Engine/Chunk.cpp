#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "Chunk.h"

namespace {

struct Vertex {
    glm::vec2 textureCoord;
    glm::u8vec3 position;
};

// How many block types there are
constexpr auto numTypes = 4;
// How many vertices per block side
constexpr auto numVertices = 6;

constexpr auto texArraySize = numTypes * numVertices;
const std::array<glm::vec2, texArraySize> texLookup {

        /* 0. GRASS SIDE */
        glm::vec2{ 0.635f, 0.9375f },
        glm::vec2{ 0.759f, 0.9375f },
        glm::vec2{ 0.635f, 1.0f },
        glm::vec2{ 0.635f, 1.0f },
        glm::vec2{ 0.759f, 0.9375f },
        glm::vec2{ 0.759f, 1.0f },

        /* 1. GRASS TOP */

        glm::vec2{ 0.507f, 0.557f },
        glm::vec2{ 0.633f, 0.557f },
        glm::vec2{ 0.507f, 0.619f },
        glm::vec2{ 0.507f, 0.619f },
        glm::vec2{ 0.633f, 0.557f },
        glm::vec2{ 0.633f, 0.619f },

        /* 2. DIRT */

        glm::vec2{ 0.634f, 0.875f },
        glm::vec2{ 0.759f, 0.875f },
        glm::vec2{ 0.634f, 0.936f },
        glm::vec2{ 0.634f, 0.936f },
        glm::vec2{ 0.759f, 0.875f },
        glm::vec2{ 0.759f, 0.936f },

        /* 3. STONE */

        glm::vec2{ 0.254f, 0.62f },
        glm::vec2{ 0.379f, 0.62f },
        glm::vec2{ 0.254f, 0.683f },
        glm::vec2{ 0.254f, 0.683f },
        glm::vec2{ 0.379f, 0.62f },
        glm::vec2{ 0.379f, 0.683f },
};

}

namespace Engine
{

std::size_t atlasLookup(BlockType type, BlockSide side)
{
    switch (type) {
        case Engine::BlockType::GRASS:
            switch(side) {
                case Engine::BlockSide::SIDE: return 0 * 6;
                case Engine::BlockSide::TOP: return 1 * 6;
                case Engine::BlockSide::BOTTOM: return 2 * 6;
            }
        case Engine::BlockType::DIRT: return 2*6;
        case Engine::BlockType::STONE: return 3*6;
    }
    return 0;
}

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
    std::vector<Vertex> vertices;
    vertices.reserve(ChunkData::BLOCKS * 6 * 6);

    for(auto x = 0; x < ChunkData::BLOCKS_X; x++) {
        for (auto y = 0; y < ChunkData::BLOCKS_Y; y++) {
            for (auto z = 0; z < ChunkData::BLOCKS_Z; z++) {

                const BlockType typ = get(x,y,z);

                if (typ == BlockType::AIR){
                    continue;
                }

                const auto offset_side = atlasLookup(typ, BlockSide::SIDE);
                const auto offset_top = atlasLookup(typ, BlockSide::TOP);
                const auto offset_bottom = atlasLookup(typ, BlockSide::BOTTOM);

                // - X
                auto negXNeighbor = neighbor(Direction::NegX);
                bool negXCond = x != 0 ? get(x-1,y,z) == BlockType::AIR : (negXNeighbor ? negXNeighbor->get(ChunkData::BLOCKS_X - 1, y, z) == BlockType::AIR : true);
                if (negXCond) {
                    vertices.emplace_back(Vertex { texLookup[offset_side + 0], glm::vec3(x, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 1], glm::vec3(x, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 2], glm::vec3(x, y + 1, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 3], glm::vec3(x, y + 1, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 4], glm::vec3(x, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 5], glm::vec3(x, y + 1, z + 1) });
                }

                // + X
                auto posXNeighbor = neighbor(Direction::PlusX);
                bool posXCond = x != ChunkData::BLOCKS_X - 1 ? get(x+1,y,z) == BlockType::AIR : (posXNeighbor ? posXNeighbor->get(0, y, z) == BlockType::AIR : true);
                if (posXCond) {
                    vertices.emplace_back(Vertex { texLookup[offset_side + 0], glm::vec3(x + 1, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 1], glm::vec3(x + 1, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 2], glm::vec3(x + 1, y + 1, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 3], glm::vec3(x + 1, y + 1, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 4], glm::vec3(x + 1, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 5], glm::vec3(x + 1, y + 1, z) });
                }

                // - Y
                auto negYNeighbor = neighbor(Direction::NegY);
                bool negYCond = y != 0 ? get(x,y-1,z) == BlockType::AIR : (negYNeighbor ? negYNeighbor->get(x, ChunkData::BLOCKS_Y - 1, z) == BlockType::AIR : true);
                if (negYCond) {
                    vertices.emplace_back(Vertex { texLookup[offset_bottom + 0], glm::vec3(x, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_bottom + 1], glm::vec3(x, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_bottom + 2], glm::vec3(x + 1, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_bottom + 3], glm::vec3(x + 1, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_bottom + 4], glm::vec3(x, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_bottom + 5], glm::vec3(x + 1, y, z) });
                }

                // + Y
                auto posYNeighbor = neighbor(Direction::PlusY);
                bool posYCond = y != ChunkData::BLOCKS_Y - 1 ? get(x,y+1,z) == BlockType::AIR : (posYNeighbor ? posYNeighbor->get(x, 0, z) == BlockType::AIR : true);
                if (posYCond) {
                    vertices.emplace_back(Vertex { texLookup[offset_top + 0], glm::vec3(x, y + 1, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_top + 1], glm::vec3(x, y + 1, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_top + 2], glm::vec3(x + 1, y + 1, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_top + 3], glm::vec3(x + 1, y + 1, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_top + 4], glm::vec3(x, y + 1, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_top + 5], glm::vec3(x + 1, y + 1, z + 1) });
                }

                // - Z
                auto negZNeighbor = neighbor(Direction::NegZ);
                bool negZCond = z != 0 ? get(x,y,z-1) == BlockType::AIR : (negZNeighbor ? negZNeighbor->get(x, y, ChunkData::BLOCKS_Z - 1) == BlockType::AIR : true);
                if (negZCond) {
                    vertices.emplace_back(Vertex { texLookup[offset_side + 0], glm::vec3(x + 1, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 1], glm::vec3(x, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 2], glm::vec3(x + 1, y + 1, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 3], glm::vec3(x + 1, y + 1, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 4], glm::vec3(x, y, z) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 5], glm::vec3(x, y + 1, z) });
                }

                // + Z
                auto posZNeighbor = neighbor(Direction::PlusZ);
                bool posZCond = z != ChunkData::BLOCKS_Z - 1 ? get(x,y,z+1) == BlockType::AIR : (posZNeighbor ? posZNeighbor->get(x, y, 0) == BlockType::AIR : true);
                if (posZCond) {
                    vertices.emplace_back(Vertex { texLookup[offset_side + 0], glm::vec3(x, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 1], glm::vec3(x + 1, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 2], glm::vec3(x, y + 1, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 3], glm::vec3(x, y + 1, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 4], glm::vec3(x + 1, y, z + 1) });
                    vertices.emplace_back(Vertex { texLookup[offset_side + 5], glm::vec3(x + 1, y + 1, z + 1) });
                }
            }
        }
    }

    m_vertices = vertices.size();

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

}
}

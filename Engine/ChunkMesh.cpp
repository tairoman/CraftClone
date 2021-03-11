#include "ChunkMesh.h"
#include "Chunk.h"

using namespace Engine;

namespace
{

// How many block types there are
constexpr auto numTypes = 4;
// How many vertices per block side
constexpr auto numVertices = 6;

constexpr auto texArraySize = numTypes * numVertices;
constexpr std::array<glm::vec2, texArraySize> texLookup {

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

std::size_t atlasLookup(BlockType type, BlockSide side)
{
    switch (type) {
        case BlockType::GRASS:
            switch(side) {
                case BlockSide::SIDE: return 0 * 6;
                case BlockSide::TOP: return 1 * 6;
                case BlockSide::BOTTOM: return 2 * 6;
            }
        case BlockType::DIRT: return 2*6;
        case BlockType::STONE: return 3*6;
    }
    return 0;
}

}

ChunkMesh::ChunkMesh(Chunk* chunk)
    : m_chunk(chunk)
{
    m_vertices.reserve(ChunkData::BLOCKS * 6 * 6);
}

const std::vector<Vertex>& ChunkMesh::vertices() const
{
    return m_vertices;
}

void ChunkMesh::regenerate()
{
    m_vertices.clear();

    for(auto x = 0; x < ChunkData::BLOCKS_X; x++) {
        for (auto y = 0; y < ChunkData::BLOCKS_Y; y++) {
            for (auto z = 0; z < ChunkData::BLOCKS_Z; z++) {

                const BlockType typ = m_chunk->get(x,y,z);

                if (typ == BlockType::AIR){
                    continue;
                }

                const auto offset_side = atlasLookup(typ, BlockSide::SIDE);
                const auto offset_top = atlasLookup(typ, BlockSide::TOP);
                const auto offset_bottom = atlasLookup(typ, BlockSide::BOTTOM);

                // - X
                auto negXNeighbor = m_chunk->neighbor(Direction::NegX);
                bool negXCond = x != 0 ? m_chunk->get(x-1,y,z) == BlockType::AIR : (negXNeighbor ? negXNeighbor->get(ChunkData::BLOCKS_X - 1, y, z) == BlockType::AIR : true);
                if (negXCond) {
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 0), glm::vec3(x, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 1), glm::vec3(x, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 2), glm::vec3(x, y + 1, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 3), glm::vec3(x, y + 1, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 4), glm::vec3(x, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 5), glm::vec3(x, y + 1, z + 1) });
                }

                // + X
                auto posXNeighbor = m_chunk->neighbor(Direction::PlusX);
                bool posXCond = x != ChunkData::BLOCKS_X - 1 ? m_chunk->get(x+1,y,z) == BlockType::AIR : (posXNeighbor ? posXNeighbor->get(0, y, z) == BlockType::AIR : true);
                if (posXCond) {
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 0), glm::vec3(x + 1, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 1), glm::vec3(x + 1, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 2), glm::vec3(x + 1, y + 1, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 3), glm::vec3(x + 1, y + 1, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 4), glm::vec3(x + 1, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 5), glm::vec3(x + 1, y + 1, z) });
                }

                // - Y
                auto negYNeighbor = m_chunk->neighbor(Direction::NegY);
                bool negYCond = y != 0 ? m_chunk->get(x,y-1,z) == BlockType::AIR : (negYNeighbor ? negYNeighbor->get(x, ChunkData::BLOCKS_Y - 1, z) == BlockType::AIR : true);
                if (negYCond) {
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_bottom + 0), glm::vec3(x, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_bottom + 1), glm::vec3(x, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_bottom + 2), glm::vec3(x + 1, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_bottom + 3), glm::vec3(x + 1, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_bottom + 4), glm::vec3(x, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_bottom + 5), glm::vec3(x + 1, y, z) });
                }

                // + Y
                auto posYNeighbor = m_chunk->neighbor(Direction::PlusY);
                bool posYCond = y != ChunkData::BLOCKS_Y - 1 ? m_chunk->get(x,y+1,z) == BlockType::AIR : (posYNeighbor ? posYNeighbor->get(x, 0, z) == BlockType::AIR : true);
                if (posYCond) {
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_top + 0), glm::vec3(x, y + 1, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_top + 1), glm::vec3(x, y + 1, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_top + 2), glm::vec3(x + 1, y + 1, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_top + 3), glm::vec3(x + 1, y + 1, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_top + 4), glm::vec3(x, y + 1, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_top + 5), glm::vec3(x + 1, y + 1, z + 1) });
                }

                // - Z
                auto negZNeighbor = m_chunk->neighbor(Direction::NegZ);
                bool negZCond = z != 0 ? m_chunk->get(x,y,z-1) == BlockType::AIR : (negZNeighbor ? negZNeighbor->get(x, y, ChunkData::BLOCKS_Z - 1) == BlockType::AIR : true);
                if (negZCond) {
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 0), glm::vec3(x + 1, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 1), glm::vec3(x, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 2), glm::vec3(x + 1, y + 1, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 3), glm::vec3(x + 1, y + 1, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 4), glm::vec3(x, y, z) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 5), glm::vec3(x, y + 1, z) });
                }

                // + Z
                auto posZNeighbor = m_chunk->neighbor(Direction::PlusZ);
                bool posZCond = z != ChunkData::BLOCKS_Z - 1 ? m_chunk->get(x,y,z+1) == BlockType::AIR : (posZNeighbor ? posZNeighbor->get(x, y, 0) == BlockType::AIR : true);
                if (posZCond) {
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 0), glm::vec3(x, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 1), glm::vec3(x + 1, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 2), glm::vec3(x, y + 1, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 3), glm::vec3(x, y + 1, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 4), glm::vec3(x + 1, y, z + 1) });
                    m_vertices.emplace_back(Vertex { texLookup.at(offset_side + 5), glm::vec3(x + 1, y + 1, z + 1) });
                }
            }
        }
    }
}

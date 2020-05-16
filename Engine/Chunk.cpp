#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "Chunk.h"

constexpr auto X_BLK_OFFSET = 0;
constexpr auto Y_BLK_OFFSET = ChunkData::BLOCKS_X;
constexpr auto Z_BLK_OFFSET = ChunkData::BLOCKS_X * ChunkData::BLOCKS_Y;

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
    const std::array<glm::vec2, texArraySize> texLookup{

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
    };

    Chunk::Chunk(glm::vec3 pos, GLuint texture, BlockType typ)
        : modelWorldMatrix(glm::translate(glm::mat4{1.0f}, pos))
        , startPos(pos)
    {

        glGenBuffers(1, &this->vbo);
        glGenVertexArrays(1, &this->vao);

        glBindVertexArray(this->vao);

        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoord));
        glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        for (auto& blk : this->blocks) {
            blk = typ;
        }

        neighbors.fill(nullptr);

        this->texture = texture;

    }

    Chunk::~Chunk()
    {
        glDeleteBuffers(1, &this->vbo);
        glDeleteVertexArrays(1, &this->vao);
    }

    BlockType Chunk::get(int x, int y, int z) const
    {
        return this->blocks[X_BLK_OFFSET * x + Y_BLK_OFFSET * y + Z_BLK_OFFSET * z];
    }

    void Chunk::set(int x, int y, int z, BlockType type)
    {
        this->changed = true;
        this->blocks[X_BLK_OFFSET * x + Y_BLK_OFFSET * y + Z_BLK_OFFSET * z] = type;
    }

    void Chunk::render()
    {
        if (this->changed){
            this->updateVbo();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->texture);

        glBindVertexArray(this->vao);

        glDrawArrays(GL_TRIANGLES, 0, this->vertices);
    }

    void Chunk::setNeighbor(Chunk* chunk, Direction dir)
    {
        neighbors[(std::size_t)dir] = chunk;
    }

    Chunk* Chunk::neighbor(Direction dir)
    {
        return neighbors[std::size_t(dir)];
    }

    glm::vec3 Chunk::getCenterPos() const
    {
        return glm::vec3
        {
            startPos.x + ChunkData::BLOCKS_X * ChunkData::BLOCK_WORLD_EXTENT / 2.0f,
            startPos.y + ChunkData::BLOCKS_Y * ChunkData::BLOCK_WORLD_EXTENT / 2.0f,
            startPos.z + ChunkData::BLOCKS_Z * ChunkData::BLOCK_WORLD_EXTENT / 2.0f
        };
    }

    void Chunk::updateVbo()
    {

        this->changed = false;
        std::vector<Vertex> vertices;
        vertices.reserve(ChunkData::BLOCKS_X * ChunkData::BLOCKS_Y * ChunkData::BLOCKS_Z * 6 * 6);

        for(auto z = 0; z < ChunkData::BLOCKS_Z; ++z) {
            for (auto y = 0; y < ChunkData::BLOCKS_Y; ++y) {
                for (auto x = 0; x < ChunkData::BLOCKS_X; ++x) {

                    const BlockType typ = this->get(x,y,z);

                    if (typ == BlockType::AIR){
                        continue;
                    }

                    const auto offset_side = atlasLookup(typ, BlockSide::SIDE);
                    const auto offset_top = atlasLookup(typ, BlockSide::TOP);
                    const auto offset_bottom = atlasLookup(typ, BlockSide::BOTTOM);

                    // - X
                    auto negXNeighbor = neighbor(Direction::NegX);
                    bool negXCond = x != 0 ? this->get(x-1,y,z) == BlockType::AIR : (negXNeighbor ? negXNeighbor->get(ChunkData::BLOCKS_X - 1, y, z) == BlockType::AIR : true);
                    //(x == 0 && negXNeighbor && negXNeighbor->get(ChunkData::BLOCKS_X - 1, y, z) == BlockType::AIR) 
                                    // this->get(x-1,y,z) == BlockType::AIR
                                    // || (x == 0 && negXNeighbor == nullptr);
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
                    bool posXCond = x != ChunkData::BLOCKS_X - 1 ? this->get(x+1,y,z) == BlockType::AIR : (posXNeighbor ? posXNeighbor->get(0, y, z) == BlockType::AIR : true);
                    //x == ChunkData::BLOCKS_X - 1;
                    //(x == ChunkData::BLOCKS_X - 1 && posXNeighbor && posXNeighbor->get(0, y, z) == BlockType::AIR) 
                                    // this->get(x+1,y,z) == BlockType::AIR
                                    // || (x == ChunkData::BLOCKS_X - 1 && posXNeighbor == nullptr);
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
                    bool negYCond = y != 0 ? this->get(x,y-1,z) == BlockType::AIR : (negYNeighbor ? negYNeighbor->get(x, ChunkData::BLOCKS_Y - 1, z) == BlockType::AIR : true);
                    //(y == 0 && negYNeighbor && negYNeighbor->get(x, ChunkData::BLOCKS_Y - 1, z) == BlockType::AIR) 
                                    //  this->get(x,y-1,z) == BlockType::AIR
                                    // || (y == 0 && negYNeighbor == nullptr);
                    if (negYCond) {
                        vertices.emplace_back(Vertex { texLookup[offset_side + 0], glm::vec3(x, y, z + 1) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 1], glm::vec3(x, y, z) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 2], glm::vec3(x + 1, y, z + 1) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 3], glm::vec3(x + 1, y, z + 1) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 4], glm::vec3(x, y, z) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 5], glm::vec3(x + 1, y, z) });
                    }

                    // + Y
                    auto posYNeighbor = neighbor(Direction::PlusY);
                    bool posYCond = y != ChunkData::BLOCKS_Y - 1 ? this->get(x,y+1,z) == BlockType::AIR : (posYNeighbor ? posYNeighbor->get(x, 0, z) == BlockType::AIR : true);
                    // (y == ChunkData::BLOCKS_Y - 1 && posYNeighbor && posYNeighbor->get(x, 0, z) == BlockType::AIR) 
                    //                  this->get(x,y+1,z) == BlockType::AIR
                                    //|| (y == ChunkData::BLOCKS_Y - 1 && posYNeighbor == nullptr);
                    if (posYCond) {
                        vertices.emplace_back(Vertex { texLookup[offset_side + 0], glm::vec3(x, y + 1, z) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 1], glm::vec3(x, y + 1, z + 1) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 2], glm::vec3(x + 1, y + 1, z) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 3], glm::vec3(x + 1, y + 1, z) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 4], glm::vec3(x, y + 1, z + 1) });
                        vertices.emplace_back(Vertex { texLookup[offset_side + 5], glm::vec3(x + 1, y + 1, z + 1) });
                    }

                    // - Z
                    auto negZNeighbor = neighbor(Direction::NegZ);
                    bool negZCond = z != 0 ? this->get(x,y,z-1) == BlockType::AIR : (negZNeighbor ? negZNeighbor->get(x, y, ChunkData::BLOCKS_Z - 1) == BlockType::AIR : true);
                    //(z == 0 && negZNeighbor && negZNeighbor->get(x, y, ChunkData::BLOCKS_Z - 1) == BlockType::AIR) 
                                    //  this->get(x,y,z-1) == BlockType::AIR
                                    // || (z == 0 && negZNeighbor == nullptr);
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
                    bool posZCond = z != ChunkData::BLOCKS_Z - 1 ? this->get(x,y,z+1) == BlockType::AIR : (posZNeighbor ? posZNeighbor->get(x, y, 0) == BlockType::AIR : true);
                    //(z == ChunkData::BLOCKS_Z - 1 && posZNeighbor && posZNeighbor->get(x, y, z+1) == BlockType::AIR) 
                                    //  this->get(x,y,z+1) == BlockType::AIR
                                    // || (z == ChunkData::BLOCKS_Z - 1 && posZNeighbor == nullptr);
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

        this->vertices = vertices.size();

        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    }
}

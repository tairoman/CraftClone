#include <cstdint>

#include <GL/glew.h>

#include "Chunk.h"

constexpr auto X_BLK_OFFSET = 0;
constexpr auto Y_BLK_OFFSET = ChunkData::BLOCKS_X;
constexpr auto Z_BLK_OFFSET = ChunkData::BLOCKS_X * ChunkData::BLOCKS_Y;

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

    using vertex_data = glm::u8vec4;

    Chunk::Chunk(glm::vec3 pos, GLuint texture)
        : Chunk(pos, texture, BlockType::AIR)
    {}

    Chunk::Chunk(glm::vec3 pos, GLuint texture, BlockType typ)
        : modelWorldMatrix(glm::translate(glm::mat4{1.0f}, pos))
        , startPos(pos)
    {

        glGenBuffers(1, &this->vbo);
        glGenVertexArrays(1, &this->vao);

        glBindVertexArray(this->vao);

        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

        glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        for (auto& blk : this->blocks) {
            blk = typ;
        }

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

    glm::vec3 Chunk::getCenterPos() const
    {
        return glm::vec3
        {
            startPos.x + ChunkData::BLOCKS_X * ChunkData::BLOCK_WORLD_EXTENT / 2.0f,
            startPos.y + ChunkData::BLOCKS_Y * ChunkData::BLOCK_WORLD_EXTENT / 2.0f,
            startPos.z + ChunkData::BLOCKS_Z * ChunkData::BLOCK_WORLD_EXTENT / 2.0f
        };
    }

    /*bool Chunk::checkWorldPositionIf(int x, int y, int z, bool predicate) const
    {
        return predicate ? parent->get(x, y, z) : true;
    }*/

    void Chunk::updateVbo()
    {

        this->changed = false;
        std::array<vertex_data, ChunkData::BLOCKS_X * ChunkData::BLOCKS_Y * ChunkData::BLOCKS_Z * 6 * 6> vertices{};
        unsigned int i = 0;

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
                    if (x == 0 || this->get(x-1,y,z) == BlockType::AIR) {
                        vertices[i++] = vertex_data(x, y, z, offset_side + 0);
                        vertices[i++] = vertex_data(x, y, z + 1, offset_side + 1);
                        vertices[i++] = vertex_data(x, y + 1, z, offset_side + 2);
                        vertices[i++] = vertex_data(x, y + 1, z, offset_side + 3);
                        vertices[i++] = vertex_data(x, y, z + 1, offset_side + 4);
                        vertices[i++] = vertex_data(x, y + 1, z + 1, offset_side + 5);
                    }

                    // + X
                    if (x == ChunkData::BLOCKS_X - 1 || this->get(x+1,y,z) == BlockType::AIR) {
                        vertices[i++] = vertex_data(x + 1, y, z + 1, offset_side + 0);
                        vertices[i++] = vertex_data(x + 1, y, z, offset_side + 1);
                        vertices[i++] = vertex_data(x + 1, y + 1, z + 1, offset_side + 2);
                        vertices[i++] = vertex_data(x + 1, y + 1, z + 1, offset_side + 3);
                        vertices[i++] = vertex_data(x + 1, y, z, offset_side + 4);
                        vertices[i++] = vertex_data(x + 1, y + 1, z, offset_side + 5);
                    }

                    // - Y
                    if (y == 0 || this->get(x,y-1,z) == BlockType::AIR) {
                        vertices[i++] = vertex_data(x, y, z + 1, offset_bottom + 0);
                        vertices[i++] = vertex_data(x, y, z, offset_bottom + 1);
                        vertices[i++] = vertex_data(x + 1, y, z + 1, offset_bottom + 2);
                        vertices[i++] = vertex_data(x + 1, y, z + 1, offset_bottom + 3);
                        vertices[i++] = vertex_data(x, y, z, offset_bottom + 4);
                        vertices[i++] = vertex_data(x + 1, y, z, offset_bottom + 5);
                    }

                    // + Y
                    if (y == ChunkData::BLOCKS_Y - 1 || this->get(x,y+1,z) == BlockType::AIR) {
                        vertices[i++] = vertex_data(x, y + 1, z, offset_top + 0);
                        vertices[i++] = vertex_data(x, y + 1, z + 1, offset_top + 1);
                        vertices[i++] = vertex_data(x + 1, y + 1, z, offset_top + 2);
                        vertices[i++] = vertex_data(x + 1, y + 1, z, offset_top + 3);
                        vertices[i++] = vertex_data(x, y + 1, z + 1, offset_top + 4);
                        vertices[i++] = vertex_data(x + 1, y + 1, z + 1, offset_top + 5);
                    }

                    // - Z
                    if (z == 0 || this->get(x,y,z-1) == BlockType::AIR) {
                        vertices[i++] = vertex_data(x + 1, y, z, offset_side + 0);
                        vertices[i++] = vertex_data(x, y, z, offset_side + 1);
                        vertices[i++] = vertex_data(x + 1, y + 1, z, offset_side + 2);
                        vertices[i++] = vertex_data(x + 1, y + 1, z, offset_side + 3);
                        vertices[i++] = vertex_data(x, y, z, offset_side + 4);
                        vertices[i++] = vertex_data(x, y + 1, z, offset_side + 5);
                    }

                    // + Z
                    if (z == ChunkData::BLOCKS_Z - 1 || this->get(x,y,z+1) == BlockType::AIR) {
                        vertices[i++] = vertex_data(x, y, z + 1, offset_side + 0);
                        vertices[i++] = vertex_data(x + 1, y, z + 1, offset_side + 1);
                        vertices[i++] = vertex_data(x, y + 1, z + 1, offset_side + 2);
                        vertices[i++] = vertex_data(x, y + 1, z + 1, offset_side + 3);
                        vertices[i++] = vertex_data(x + 1, y, z + 1, offset_side + 4);
                        vertices[i++] = vertex_data(x + 1, y + 1, z + 1, offset_side + 5);
                    }
                }
            }
        }

        this->vertices = i;

        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(GL_ARRAY_BUFFER, i * sizeof(vertex_data), &vertices[0], GL_STATIC_DRAW);

    }
}

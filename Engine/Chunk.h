
#ifndef CRAFTBONE_CHUNK_H
#define CRAFTBONE_CHUNK_H

#include <array>

#include <GL/gl.h>
#include <glm/detail/qualifier.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ChunkData
{
    constexpr inline auto BLOCK_WORLD_EXTENT = 1; // The size of a block in every direction in world space 
    constexpr inline auto BLOCKS_X = 16;
    constexpr inline auto BLOCKS_Y = 128;
    constexpr inline auto BLOCKS_Z = 16;
    constexpr inline auto BLOCKS = BLOCKS_X * BLOCKS_Y * BLOCKS_Z;
}

namespace Engine
{

    enum class BlockType
    {
        AIR = 8,
        GRASS = 9,
        DIRT = 10,
        STONE = 11,
    };

    enum class Direction
    {
        NegX = 0,
        PlusX = 1,
        NegY = 2,
        PlusY = 3,
        NegZ = 4,
        PlusZ = 5,
    };

    enum class BlockSide
    {
        TOP, BOTTOM, SIDE
    };

    class Chunk
    {
    public:

        Chunk(glm::vec3 pos, GLuint texture, BlockType type = BlockType::AIR);
        ~Chunk();

        BlockType get(int x, int y, int z) const;
        void set(int x, int y, int z, BlockType type);

        const glm::mat4& getModelWorldMatrix() const {
            return m_modelWorldMatrix;
        }

        void setNeighbor(Chunk* chunk, Direction dir);
        Chunk* neighbor(Direction dir);

        void render();

        glm::vec3 getCenterPos() const;

    private:

        void updateVbo();
        bool checkWorldPositionIf(int x, int y, int z, bool predicate) const;

        glm::mat4 m_modelWorldMatrix;
        glm::vec3 m_startPos; // A corner of the chunk from which we construct all vertex positions
        std::array<BlockType, ChunkData::BLOCKS> m_blocks;
        std::array<Chunk*, 6> m_neighbors;
        GLuint m_vbo;
        GLuint m_vao;
        GLuint m_texture;
        bool m_changed = true;
        std::size_t m_vertices = 0;

    };
}


#endif //CRAFTBONE_CHUNK_H


#ifndef CRAFTBONE_CHUNK_H
#define CRAFTBONE_CHUNK_H

#include <array>

#include <GL/gl.h>
#include <glm/detail/qualifier.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include "World.h"

namespace ChunkData
{
    constexpr auto BLOCK_WORLD_EXTENT = 1; // The size of a block in every direction in world space 
    constexpr auto BLOCKS_X = 16;
    constexpr auto BLOCKS_Y = 128;
    constexpr auto BLOCKS_Z = 16;
    constexpr auto BLOCKS = BLOCKS_X * BLOCKS_Y * BLOCKS_Z;
}

namespace Engine
{

    enum class BlockType : uint8_t
    {
        AIR,
        GRASS,
        DIRT,
        STONE,
    };

    enum class BlockSide
    {
        TOP, BOTTOM, SIDE
    };

    class Chunk
    {
    public:

        Chunk(glm::vec3 pos, GLuint glTextureFogSGIX);
        Chunk(glm::vec3 pos, GLuint texture, BlockType type);
        ~Chunk();

        BlockType get(int x, int y, int z) const;
        void set(int x, int y, int z, BlockType type);

        const glm::mat4& getModelWorldMatrix() const {
            return modelWorldMatrix;
        }

        void render();

        glm::vec3 getCenterPos() const;

    private:

        void updateVbo();
        bool checkWorldPositionIf(int x, int y, int z, bool predicate) const;

        glm::mat4 modelWorldMatrix;
        glm::vec3 startPos; // A corner of the chunk from which we construct all vertex positions
        std::array<BlockType, ChunkData::BLOCKS> blocks;
        GLuint vbo;
        GLuint vao;
        GLuint texture;
        bool changed = true;
        std::size_t vertices = 0;

    };
}


#endif //CRAFTBONE_CHUNK_H

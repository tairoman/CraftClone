
#ifndef CRAFTBONE_CHUNK_H
#define CRAFTBONE_CHUNK_H

#include <array>

#include <GL/gl.h>
#include <glm/detail/qualifier.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ChunkData {
    constexpr auto BLOCKS_X = 16;
    constexpr auto BLOCKS_Y = 128;
    constexpr auto BLOCKS_Z = 16;
}

namespace Engine {

enum class BlockType : uint8_t {
    AIR,
    GRASS,
    DIRT,
    STONE,
};

enum class BlockSide {
    TOP, BOTTOM, SIDE
};

class Chunk {

    template <typename T>
    using ArrX = std::array<T, ChunkData::BLOCKS_X>;
    template <typename T>
    using ArrY = std::array<T, ChunkData::BLOCKS_Y>;
    template <typename T>
    using ArrZ = std::array<T, ChunkData::BLOCKS_Z>;
    
    using BlockArray = ArrX<ArrY<ArrZ<BlockType>>>;

public:

    Chunk(glm::vec3 pos, GLuint texture);
    ~Chunk();

    BlockType get(int x, int y, int z) const;
    void set(int x, int y, int z, BlockType type);

    const glm::mat4& getModelWorldMatrix() const {
        return modelWorldMatrix;
    }

    void render();

private:

    void updateVbo();

    glm::mat4 modelWorldMatrix{1.0f};
    BlockArray blocks{};
    GLuint vbo;
    GLuint vao;
    GLuint texture;
    bool changed = true;
    unsigned int vertices = 0;

};

}


#endif //CRAFTBONE_CHUNK_H

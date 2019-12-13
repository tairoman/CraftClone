
#ifndef CRAFTBONE_CHUNK_H
#define CRAFTBONE_CHUNK_H

#include <GL/gl.h>
#include <glm/detail/qualifier.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define BLK_SIZE_X 16
#define BLK_SIZE_Y 128
#define BLK_SIZE_Z 16

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

public:

    Chunk(glm::vec3 pos, GLuint texture);
    ~Chunk();

    BlockType get(int x, int y, int z);
    void set(int x, int y, int z, BlockType type);

    glm::mat4 modelWorldMatrix = glm::mat4(1.0f);

    void render();

private:

    void updateVbo();

    BlockType blocks[BLK_SIZE_X][BLK_SIZE_Y][BLK_SIZE_Z];
    GLuint vbo;
    GLuint vao;
    GLuint texture;
    bool changed;
    unsigned int vertices;

};

}


#endif //CRAFTBONE_CHUNK_H

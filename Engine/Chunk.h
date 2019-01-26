
#ifndef CRAFTBONE_CHUNK_H
#define CRAFTBONE_CHUNK_H

#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define SIZE_X 16
#define SIZE_Y 128
#define SIZE_Z 16

namespace Engine {

enum class BlockType : uint8_t {
    AIR,
    GRASS,
    STONE,
};

class Chunk {

public:

    Chunk(glm::vec3 pos);
    ~Chunk();

    BlockType get(int x, int y, int z);
    void set(int x, int y, int z, BlockType type);

    glm::mat4 modelWorldMatrix = glm::mat4(1.0f);

    void render();

private:

    void updateVbo();
    bool anyAirNeighbor(int x, int y, int z);

    BlockType blocks[SIZE_X][SIZE_Y][SIZE_Z];
    GLuint vbo;
    GLuint vao;
    bool changed;
    unsigned int vertices;

};

}


#endif //CRAFTBONE_CHUNK_H

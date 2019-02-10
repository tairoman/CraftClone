
#include <GL/glew.h>

#include "Chunk.h"

namespace Engine {

using vertex_data = glm::u8vec4;

Chunk::Chunk(glm::vec3 pos) {

    this->modelWorldMatrix = glm::translate(this->modelWorldMatrix, pos);

    this->changed = true;
    glGenBuffers(1, &this->vbo);
    glGenVertexArrays(1, &this->vao);

    glBindVertexArray(this->vao);

    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    for(int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                this->set(x,y,z, BlockType::AIR);
            }
        }
    }
}

Chunk::~Chunk() {
    glDeleteBuffers(1, &this->vbo);
    glDeleteVertexArrays(1, &this->vao);
}

BlockType Chunk::get(int x, int y, int z) {
    return this->blocks[x][y][z];
}

void Chunk::set(int x, int y, int z, BlockType type) {
    this->changed = true;
    this->blocks[x][y][z] = type;
}

void Chunk::render() {
    if (this->changed)
        this->updateVbo();

    glBindVertexArray(this->vao);

    glDrawArrays(GL_TRIANGLES, 0, this->vertices);
}

void Chunk::updateVbo() {

    this->changed = false;
    vertex_data vertices[SIZE_X * SIZE_Y * SIZE_Z * 6 * 6];
    unsigned int i = 0;

    for(int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {

                BlockType typ = this->get(x,y,z);

                if (typ == BlockType::AIR){
                    continue;
                }

                uint8_t type = static_cast<uint8_t>(typ);

                // - X

                if (x == 0 || this->get(x-1,y,z) == BlockType::AIR) {
                    vertices[i++] = vertex_data(x, y, z, type);
                    vertices[i++] = vertex_data(x, y, z + 1, type);
                    vertices[i++] = vertex_data(x, y + 1, z, type);
                    vertices[i++] = vertex_data(x, y + 1, z, type);
                    vertices[i++] = vertex_data(x, y, z + 1, type);
                    vertices[i++] = vertex_data(x, y + 1, z + 1, type);
                }

                // + X
                if (x == SIZE_X - 1 || this->get(x+1,y,z) == BlockType::AIR) {
                    vertices[i++] = vertex_data(x + 1, y, z, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z, type);
                    vertices[i++] = vertex_data(x + 1, y, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y, z + 1, type);
                }

                // - Y
                if (y == 0 || this->get(x,y-1,z) == BlockType::AIR) {
                    vertices[i++] = vertex_data(x, y, z, type);
                    vertices[i++] = vertex_data(x + 1, y, z, type);
                    vertices[i++] = vertex_data(x, y, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y, z, type);
                    vertices[i++] = vertex_data(x + 1, y, z + 1, type);
                    vertices[i++] = vertex_data(x, y, z + 1, type);
                }

                // + Y
                if (y == SIZE_Y - 1 || this->get(x,y+1,z) == BlockType::AIR) {
                    vertices[i++] = vertex_data(x, y + 1, z, type);
                    vertices[i++] = vertex_data(x, y + 1, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z, type);
                    vertices[i++] = vertex_data(x, y + 1, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z + 1, type);
                }

                // - Z
                if (z == 0 || this->get(x,y,z-1) == BlockType::AIR) {
                    vertices[i++] = vertex_data(x, y, z, type);
                    vertices[i++] = vertex_data(x, y + 1, z, type);
                    vertices[i++] = vertex_data(x + 1, y, z, type);
                    vertices[i++] = vertex_data(x, y + 1, z, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z, type);
                    vertices[i++] = vertex_data(x + 1, y, z, type);
                }

                // + Z
                if (z == SIZE_Z - 1 || this->get(x,y,z+1) == BlockType::AIR) {
                    vertices[i++] = vertex_data(x, y, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y, z + 1, type);
                    vertices[i++] = vertex_data(x, y + 1, z + 1, type);
                    vertices[i++] = vertex_data(x, y + 1, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y, z + 1, type);
                    vertices[i++] = vertex_data(x + 1, y + 1, z + 1, type);
                }
            }
        }
    }

    this->vertices = i;

    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, i * sizeof(vertex_data), vertices, GL_STATIC_DRAW);

}

}

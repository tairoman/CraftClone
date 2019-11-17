
#include <GL/glew.h>

#include "Chunk.h"

namespace Engine {

std::size_t atlasLookup(BlockType type, BlockSide side) {
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

Chunk::Chunk(glm::vec3 pos, GLuint texture) {

    this->modelWorldMatrix = glm::translate(this->modelWorldMatrix, pos);

    this->changed = true;
    glGenBuffers(1, &this->vbo);
    glGenVertexArrays(1, &this->vao);

    glBindVertexArray(this->vao);

    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    for(int x = 0; x < BLK_SIZE_X; x++) {
        for (int y = 0; y < BLK_SIZE_Y; y++) {
            for (int z = 0; z < BLK_SIZE_Z; z++) {
                this->set(x,y,z, BlockType::AIR);
            }
        }
    }

    this->texture = texture;

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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);

    glBindVertexArray(this->vao);

    glDrawArrays(GL_TRIANGLES, 0, this->vertices);
}

void Chunk::updateVbo() {

    this->changed = false;
    vertex_data vertices[BLK_SIZE_X * BLK_SIZE_Y * BLK_SIZE_Z * 6 * 6];
    unsigned int i = 0;

    for(int x = 0; x < BLK_SIZE_X; x++) {
        for (int y = 0; y < BLK_SIZE_Y; y++) {
            for (int z = 0; z < BLK_SIZE_Z; z++) {

                BlockType typ = this->get(x,y,z);

                if (typ == BlockType::AIR){
                    continue;
                }

                std::size_t offset_side = atlasLookup(this->get(x,y,z), BlockSide::SIDE);
                std::size_t offset_top = atlasLookup(this->get(x,y,z), BlockSide::TOP);
                std::size_t offset_bottom = atlasLookup(this->get(x,y,z), BlockSide::BOTTOM);

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
                if (x == BLK_SIZE_X - 1 || this->get(x+1,y,z) == BlockType::AIR) {
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
                if (y == BLK_SIZE_Y - 1 || this->get(x,y+1,z) == BlockType::AIR) {
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
                if (z == BLK_SIZE_Z - 1 || this->get(x,y,z+1) == BlockType::AIR) {
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
    glBufferData(GL_ARRAY_BUFFER, i * sizeof(vertex_data), vertices, GL_STATIC_DRAW);

}

}

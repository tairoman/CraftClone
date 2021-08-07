#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_uint3_sized.hpp>
#include <vector>

namespace Engine {
class Chunk;
}

struct Vertex
{
    glm::vec2 textureCoord;
    glm::u8vec3 position;
};

class ChunkMesh
{
public:
    ChunkMesh(Engine::Chunk* chunk);

    [[nodiscard]] const std::vector<Vertex>& vertices() const;

    void regenerate();

private:
    Engine::Chunk* const m_chunk;
    std::vector<Vertex> m_vertices;
};
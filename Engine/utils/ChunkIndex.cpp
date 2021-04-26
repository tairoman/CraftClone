#include "Chunkindex.h"
#include "../Chunk.h"

#include <glm/glm.hpp>

ChunkIndex::ChunkIndex(glm::ivec3 index)
    : m_index(std::move(index))
{
}

ChunkIndex ChunkIndex::fromWorldPos(const glm::ivec3& pos)
{
    return ChunkIndex{{
        int(std::floor(pos.x / ChunkData::BLOCKS_X)),
        int(std::floor(pos.y / ChunkData::BLOCKS_Y)),
        int(std::floor(pos.z / ChunkData::BLOCKS_Z))
    }};
}

glm::ivec3 ChunkIndex::toWorldPos() const
{
    return {
        m_index.x * ChunkData::BLOCKS_X * ChunkData::BLOCK_WORLD_EXTENT,
        m_index.y * ChunkData::BLOCKS_Y * ChunkData::BLOCK_WORLD_EXTENT,
        m_index.z * ChunkData::BLOCKS_Z * ChunkData::BLOCK_WORLD_EXTENT
    };
}

const glm::ivec3& ChunkIndex::data() const
{
    return m_index;
}

glm::ivec3 chunkViewDistance()
{
    static const auto s_chunkViewDistance = glm::ivec3{ 10, 5, 10 };
    return s_chunkViewDistance;
}

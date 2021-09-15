#pragma once

#include <glm/glm.hpp>

glm::ivec3 chunkViewDistance();

class ChunkIndex
{
public:
	ChunkIndex() = default;
	explicit ChunkIndex(glm::ivec3 index);

	static ChunkIndex fromWorldPos(const glm::ivec3& pos);
	[[nodiscard]] glm::ivec3 toWorldPos() const;

	[[nodiscard]] const glm::ivec3& data() const;

private:
	glm::ivec3 m_index = { 0,0,0 };
};
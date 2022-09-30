#pragma once

#include <gl/glew.h>
#include <glm/fwd.hpp>

#include <memory>
#include <optional>

#include "events/EventThread.h"
#include "utils/Chunkindex.h"
#include "utils/Property.h"
#include "utils/Observer.h"

#include "../lib/PerlinNoise.hpp"

class ChunkManagerThread;

namespace Engine {
	class Chunk;
	class Shader;
};

class ChunkManager
{
public:
	explicit ChunkManager(GLuint texture);
	~ChunkManager();

	Property<glm::ivec3> sourceChunk;

	bool isWithinViewDistance(Engine::Chunk* chunk, const glm::vec3& playerPos) const;
	bool isWithinViewDistance(const ChunkIndex& chunk, const ChunkIndex& playerChunk) const;
	bool isWithinViewDistance(const glm::ivec3& offset) const;

	void ensureChunkAtIndex(const ChunkIndex& index);
	void addChunkAt(const ChunkIndex& index, GLuint texture);
	Engine::Chunk* chunkAt(const ChunkIndex& index) const;

	void renderChunks(const glm::vec3& playerPos, const Engine::Shader& shader, const glm::mat4& viewProjectionMatrix);

private:
	Observer m_observer;

	std::unique_ptr<ChunkManagerThread> m_thread;
	std::optional<ChunkIndex> m_playerChunk = {};
	mutable std::mutex m_chunksMutex;

	std::unordered_map<std::size_t, std::unique_ptr<Engine::Chunk>> m_chunks;
	siv::BasicPerlinNoise<float> m_perlinNoise;

	GLuint m_texture;

	friend class ChunkManagerThread;
};
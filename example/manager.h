#pragma once

#include <vector>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "block.h"
#include "texture.h"
#include "maths.h"
#include "terrain.h"
#include "shader.h"

#include "camera.h"

class Chunk;
class Camera;

class Manager
{
public:

	static Manager& Instance()
	{
		static Manager instance;
		return instance;
	}

	// Raycast data
	struct RaycastResult
	{
		bool hit;
		BlockInfo block;
		glm::vec3 pos;
		Math::Direction normal;
	};

	void updateChunks(glm::vec3 playerPos, float dt);

	void drawChunks(const Camera& camera);

	void setBlock(glm::ivec3 pos, const Block& block, bool network = false);
	const Block& getBlock(glm::ivec3 pos);

	std::vector<BlockInfo> getBlocksInVolume(glm::vec3 pos, glm::vec3 size);
	RaycastResult raycast(glm::vec3 pos, glm::vec3 dir, float length = INFINITY);

	Shader& getShader();

private:
	//typedef std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>> ChunkContainer;
	//returning to raw pointers temp seems not change
	typedef std::unordered_map<glm::ivec2, Chunk*> ChunkContainer;


	Shader shader_;
	//Texture texture_;
	wgfx::Texture texture_;

	ChunkContainer chunks_;
	Terrain noise_;

	Manager();
	~Manager();

	Chunk* addChunk(glm::ivec2 coord);
	bool chunkInRange(glm::vec3 playerPos, glm::vec3 chunkPos) const;

	int builtNeighborCount(glm::ivec2 coord) const;
	int builtNeighborCount(glm::ivec2 coord, glm::ivec2 exclude) const;

	Chunk* getChunk(glm::ivec3 pos);
	const Chunk* getChunk(glm::ivec3 pos) const;
	Chunk* getChunk(glm::ivec2 chunkCoord);
	const Chunk* getChunk(glm::ivec2 chunkCoord) const;

	glm::ivec2 toRelativePos(glm::ivec3 pos) const;
	glm::ivec2 toChunkPos(glm::ivec3 pos) const;


public:
	Manager(Manager const&) = delete;
	void operator=(Manager const&) = delete;

};
#pragma once

#include <array>

#include "maths.h"
#include "constants.h"
#include "block.h"

#include "model.h"

#include "terrain.h"

class Chunk
{
public:

	Chunk(glm::ivec2 pos);

	void generate(Terrain& terrain);

	void buildMesh();

	void clear();

	bool meshBuilt() const;

	void setBlock(glm::ivec3 pos, const Block& block);
	const Block& getBlock(glm::ivec3 pos) const;

	glm::ivec2 getCoord() const;
	glm::ivec3 getPos() const;
	glm::vec3 getWorldPos() const;
	//glm::vec3 getRenderPos() const;

	bool isVisible(const Math::Frustum& camera) const;

	void draw(wgfx::Pipeline* pipeline);

	bool destroy = false;

	Model model_;
private:

	glm::ivec2 pos_;

	glm::vec3 worldPos_;
	int highestBlock_;

	std::array<Block, World::chunkSize* World::chunkSize* World::chunkHeight> blocks_ = {};

	glm::ivec3 worldToLocal(glm::ivec3 pos) const;
	glm::ivec3 localToWorld(glm::ivec3 pos) const;

	bool outOfBounds(glm::ivec3 pos) const;

	const Block& getBlockLocal(glm::ivec3 pos) const;
	void setBlockLocal(glm::ivec3 pos, const Block& block);

	bool checkForBlock(glm::ivec3 pos) const;
};
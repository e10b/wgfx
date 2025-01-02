
#include "chunk.h"
#include "manager.h"

#include "glm/gtc/noise.hpp"
#include "glm/gtx/compatibility.hpp"

Chunk::Chunk(glm::ivec2 pos) : pos_(pos), highestBlock_(0), model_(World::chunkArea * 8)
{
}

void Chunk::generate(Terrain& terrain)
{
	glm::ivec3 chunk_pos = getWorldPos();

	// Terrain

	// Loop over z and x and gen heights
	for (int z = 0; z < World::chunkSize; z++)
	{
		for (int x = 0; x < World::chunkSize; x++)
		{
			// Get the height for this coord
			glm::ivec2 pos = glm::ivec2(chunk_pos.x + x, chunk_pos.z + z);
			int height = terrain.getHeight(pos);
			for (int y = 0; y < height; y++)
			{
				Block block;

				// hardcoded type based on elevation
				if (y < height - 8)
					block = { Block::STONE };
				else if (y < height - 1)
					block = { Block::DIRT };
				else
					block = { Block::GRASS };

				setBlockLocal({ x,y,z }, block);
			}
		}
	}

	// Trees

	glm::ivec3 treeSize = {
		std::size(*Terrain::tree),
		std::size(Terrain::tree),
		std::size(**Terrain::tree)
	};
	glm::ivec2 treeRad = {
		(treeSize.x - 1) / 2,
		(treeSize.z - 1) / 2
	};
	glm::ivec2 chunkPos2d = {
		chunk_pos.x,
		chunk_pos.z
	};

	// Get all points of trees around this chunk
	std::vector<glm::ivec2> treePoints = terrain.generateTreePoints(
		chunkPos2d - treeRad,
		chunkPos2d + glm::ivec2(World::chunkSize, World::chunkSize) + treeRad
	);

	// For each tree, loop over all it's blocks and set them
	for (unsigned i = 0; i < treePoints.size(); i++)
	{
		for (int y = 0; y < treeSize.y; y++)
		{
			for (int x = -treeRad.x; x <= treeRad.x; x++)
			{
				for (int z = -treeRad.y; z <= treeRad.y; z++)
				{
					// Get block from tree data
					Block newBlock = { Block::BlockType(Terrain::tree[y][x + treeRad.x][z + treeRad.y]) };

					if (newBlock.type != Block::AIR)
					{
						glm::ivec3 blockPos = {
							treePoints[i].x + x,
							terrain.getHeight(treePoints[i]) + y,
							treePoints[i].y + z
						};

						const Block& oldBlock = getBlock(blockPos);

						// Only allow leaves to replace air
						if (newBlock.type != Block::LEAVES || oldBlock.type == Block::AIR)
							setBlock(blockPos, newBlock);
					}
				}
			}
		}
	}
}

void Chunk::buildMesh()
{
	model_.clear();

	// loop over all blocks
	for (int y = 0; y <= highestBlock_; y++) {
		for (int z = 0; z < World::chunkSize; z++) {
			for (int x = 0; x < World::chunkSize; x++) {
				// if not air
				const Block& block = getBlockLocal({ x,y,z });
				if (block.type == Block::AIR)
					continue;

				// for each dir
				for (int d = 0; d < Math::DIRECTION_COUNT; d++)
				{
					//get block in dir
					glm::ivec3 adj = { x,y,z };
					glm::vec3 norm = Math::directionVectors[d];
					adj += norm;

					// if block in dir
					if (adj.y >= 0 && checkForBlock(adj))
					{
						const int tilesheetSize = 8;
						unsigned index = 0;

						// get tex index
						switch (d)
						{
						case Math::DIRECTION_FORWARD:
						case Math::DIRECTION_BACKWARD:
						case Math::DIRECTION_LEFT:
						case Math::DIRECTION_RIGHT:
							index = BlockData::sideIndicies[block.type].side;
							break;
						case Math::DIRECTION_UP:
							index = BlockData::sideIndicies[block.type].top;
							break;
						case Math::DIRECTION_DOWN:
							index = BlockData::sideIndicies[block.type].bottom;
							break;
						}

						// get tex coords
						glm::vec2 offset = Math::getUVFromSheet(tilesheetSize, tilesheetSize, index, Math::CORNER_TOP_LEFT);
						//offset.y = 1.0f - offset.y - (1.0f / tilesheetSize); // flip tex

						model_.addQuad(quads[Math::Direction(d)], offset, 1.0f / tilesheetSize, { x,y,z }, getWorldPos());
					}
				}

				/*
				// for each dir
				if (checkForBlock({ x - 1, y , z })) {
					model_.addFace(LEFT_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x + 1, y , z })) {
					model_.addFace(RIGHT_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y, z + 1 })) {
					model_.addFace(FRONT_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y, z - 1 })) {
					model_.addFace(BACK_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y - 1, z })) {
					model_.addFace(BOTTOM_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y + 1, z })) {
					model_.addFace(TOP_FACE, getPos(), { x,y,z });
				}
				*/


				/*
				// for each dir
				if (checkForBlock({ x + 1, y , z })) {
					model_.addFace(RIGHT_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x - 1, y , z })) {
					model_.addFace(LEFT_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y + 1, z })) {
					model_.addFace(TOP_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y - 1, z })) {
					model_.addFace(BOTTOM_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y, z + 1 })) {
					model_.addFace(FRONT_FACE, getPos(), { x,y,z });
				}
				if (checkForBlock({ x, y, z - 1 })) {
					model_.addFace(BACK_FACE, getPos(), { x,y,z });
				}
				*/
			}
		}
	}

	model_.buffer(&model_);
}

void Chunk::clear()
{
	model_.clear();
}

bool Chunk::meshBuilt() const
{
	if (this == nullptr) {
		// Handle the case where 'this' is nullptr, e.g., by returning a default value
		return false; // or any other default value
	}
	return model_.indexCount() != 0;
}

void Chunk::setBlock(glm::ivec3 pos, const Block& block)
{
	glm::ivec3 local = worldToLocal(pos);

	if (!outOfBounds(local))
		setBlockLocal(local, block);
}

const Block& Chunk::getBlock(glm::ivec3 pos) const
{
	glm::ivec3 local = worldToLocal(pos);

	if (outOfBounds(local))
	{
		static Block error = { Block::BLOCK_ERROR };
		return error;
	}
	return getBlockLocal(local);
}

glm::ivec2 Chunk::getCoord() const
{
	return pos_;
}

glm::ivec3 Chunk::getPos() const
{
	return glm::vec3(pos_.x, 0, pos_.y);
}

glm::vec3 Chunk::getWorldPos() const
{
	if (this == nullptr) {
		// Handle the case where 'this' is nullptr, e.g., by returning a default value
		return glm::vec3(0.0f); // or any other default value
	}

	return glm::vec3(pos_.x * (float)World::chunkSize, 0, pos_.y * (float)World::chunkSize);
}

void Chunk::draw(wgfx::Pipeline* pipeline)
{
	//model_ = mesh_.createBuffer();


	if (meshBuilt())
		//mesh_.createBuffer().bind();
		model_.bind(pipeline);
}

glm::ivec3 Chunk::worldToLocal(glm::ivec3 pos) const
{
	glm::vec3 world = getWorldPos();
	return glm::ivec3(pos.x - world.x, pos.y, pos.z - world.z);
}

glm::ivec3 Chunk::localToWorld(glm::ivec3 pos) const
{
	glm::vec3 world = getWorldPos();
	return glm::ivec3(pos.x + world.x, pos.y, pos.z + world.z);
}

bool Chunk::outOfBounds(glm::ivec3 pos) const
{
	return pos.x < 0 || pos.x >= World::chunkSize ||
		pos.y < 0 || pos.y >= World::chunkHeight ||
		pos.z < 0 || pos.z >= World::chunkSize;
}

const Block& Chunk::getBlockLocal(glm::ivec3 pos) const
{
	return blocks_[pos.x + pos.y * World::chunkArea + pos.z * World::chunkSize];
}

void Chunk::setBlockLocal(glm::ivec3 pos, const Block& block)
{
	blocks_[pos.x + pos.y * World::chunkArea + pos.z * World::chunkSize] = block;

	// this operation might change the highest block
	if (pos.y > highestBlock_)
		highestBlock_ = pos.y;
}

bool Chunk::checkForBlock(glm::ivec3 pos) const
{
	if (!outOfBounds(pos))
		return getBlockLocal(pos).type == Block::AIR;
	else
	{
		// Block above/below height limits
		if (pos.y < -1)
			return false;

		// Block in another chunk
		glm::ivec3 world = localToWorld(pos);
		Block block = Manager::Instance().getBlock(world);

		//assert(block.type == Block::BLOCK_ERROR);

		return block.type == Block::AIR;

	}
}

bool Chunk::isVisible(const Math::Frustum& camera) const
{
	glm::vec3 position = getWorldPos();
	float s = 1.f;

	for (unsigned i = 0; i < std::size(camera.planes); i++)
	{
		// Frustum + aabb collision
		glm::vec3 positive = position;
		if (camera.planes[i].a >= 0)
			positive.x += World::chunkSize * s;
		if (camera.planes[i].b >= 0)
			positive.y += World::chunkHeight * s;
		if (camera.planes[i].c >= 0)
			positive.z += World::chunkSize * s;

		if (positive.x * camera.planes[i].a + positive.y * camera.planes[i].b + positive.z * camera.planes[i].c + camera.planes[i].d < 0)
			return false;
	}

	return true;
}
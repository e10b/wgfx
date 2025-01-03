#include "manager.h"

#include <iostream>

#include <glm/gtx/norm.hpp>

#include "constants.h"
#include "chunk.h"
//#include "network.h"

Manager::Manager() : shader_("shader.wgsl")//, texture_("res/textures/mangoose.png")
{
	texture_ = wgfx::loadTexture(RESOURCE_DIR "/goose.png");
	//shader_.setVar("fogAmount", 0.7f / World::renderDistance);

	wgfx::VertexBuffer* vbo = wgfx::createVertexBuffer();
	vbo->setAttribute(0, wgfx::vec3f, 0); // position
	vbo->setAttribute(1, wgfx::vec3f, 3); // color
	vbo->setAttribute(2, wgfx::vec2f, 6); // uv
	shader_.setVertexBuffer({ 0.0 });
	shader_.setIndexBuffer({ 0 }); // initial data point hmm 

	shader_.setUniform(0); // camera
	shader_.setUniform(1); // model
	
	
	shader_.setTexture(2, texture_); // tex
	shader_.setSampler(3, texture_); // sampler

	shader_.setUniform(4, 0.7f / World::renderDistance); //fog amount
	shader_.setUniform(5); //campos

	shader_.setUniform(6); //view
	shader_.setUniform(7); //proj

	//shader_.setUniform(4, 0.7f / World::renderDistance); // fog

	shader_.pipeline->init(vbo); // auto init?? well, 

}

Manager::~Manager()
{
	for (const auto& c : chunks_)
		delete c.second;
}

Chunk* Manager::addChunk(glm::ivec2 coord)
{

	Chunk* current = getChunk(coord);
	if (current == nullptr)
	{
		// gen this chunk
		current = new Chunk(coord);
		current->generate(noise_);
		chunks_[coord] = current;
	}
	else if (current->meshBuilt())
	{
		// chunk alread exists
		return current;
	}

	// gen surroudnign chunks and add
	for (unsigned i = 0; i < std::size(Math::surrounding); i++)
	{
		glm::ivec2 newCoord = coord + Math::surrounding[i];
		if (getChunk(newCoord) == nullptr)
		{
			Chunk* newChunk = new Chunk(newCoord);
			newChunk->generate(noise_);
			chunks_[newCoord] = newChunk;
		}
	}

	// build this chunk's mesh
	current->buildMesh();
	return current;
}

bool Manager::chunkInRange(glm::vec3 playerPos, glm::vec3 chunkPos) const
{
	// check if chunk is closer to player than render dist
	glm::vec3 pos = chunkPos + glm::vec3(World::chunkSize, 0.0f, World::chunkSize) / 2.f;
	float distanceSquared = glm::distance2(glm::vec2(pos.x, pos.z), glm::vec2(playerPos.x, playerPos.z));
	return distanceSquared <= World::renderDistance * World::renderDistance;
}

int Manager::builtNeighborCount(glm::ivec2 coord, glm::ivec2 exclude) const
{
	unsigned count = 0;

	// loop over surrounding chunks
	for (unsigned j = 0; j < std::size(Math::surrounding); j++)
	{
		glm::ivec2 currentCoord = coord + Math::surrounding[j];

		if (currentCoord == exclude)
			continue;

		// count chunk if it's built
		const Chunk* currentChunk = getChunk(currentCoord);
		if (currentChunk != nullptr && currentChunk->meshBuilt())
			count++;
	}
	return count;
}

int Manager::builtNeighborCount(glm::ivec2 coord) const
{
	return builtNeighborCount(coord, coord);
}

void Manager::updateChunks(glm::vec3 playerPos, float dt)
{

	unsigned loadedChunks = 0;

	// Create initial chunks around player
	glm::ivec2 playerChunkCoord = toChunkPos(glm::floor(playerPos));
	Chunk* playerChunk = getChunk(playerChunkCoord);

	// Ensure the initial chunks around the player are created
	if (playerChunk == nullptr || !playerChunk->meshBuilt())
	{
		// Create 3x3 cross shape of chunks around player
		loadedChunks++;
		addChunk(playerChunkCoord);

		for (int i = 0; i < Math::DIRECTION_COUNT; i++)
		{
			if (i == Math::DIRECTION_UP || i == Math::DIRECTION_DOWN)
				continue;

			loadedChunks++;
			addChunk(playerChunkCoord + glm::ivec2(Math::directionVectors[i].x, Math::directionVectors[i].z));
		}
	}

	// Update all chunks
	for (auto it = chunks_.begin(); it != chunks_.end();)
	{
		if (chunkInRange(playerPos, it->second->getWorldPos()))
		{
			// Build meshes of all chunks and add unmeshed ones surrounding
			if (loadedChunks < World::renderSpeed && !it->second->meshBuilt() && builtNeighborCount(it->first) >= 3)
			{
				loadedChunks++;
				addChunk(it->first);
			}
			++it;
		}
		else
		{
			// Delete surrounding chunks unconnected otherwise
			for (unsigned i = 0; i < std::size(Math::surrounding); i++)
			{
				glm::ivec2 newCoord = it->first + Math::surrounding[i];
				auto chunk = chunks_.find(newCoord);

				if (chunk != chunks_.end() && !chunk->second->meshBuilt() && builtNeighborCount(newCoord, it->first) == 0)
				{
					delete chunk->second;
					chunks_.erase(chunk);
				}
			}

			// Only remove mesh of chunk if it has neighbors
			if (builtNeighborCount(it->first) > 0)
			{
				//std::cout << it->second->model_.vbo_->id << "\n";
				it->second->clear();
				

				++it;
			}
			else
			{
				/*auto temp = it;
				++it;
				delete temp->second;
				chunks_.erase(temp);*/
				it->second->clear();
				

				delete it->second;
				it = chunks_.erase(it);
			}
		}
	}
}

void Manager::drawChunks(const Camera& camera)
{
	//uh
		//shader_.loadMatrix(camera);

		//Math::Frustum frustum = Math::calculateFrustum(camera.getViewMatrix());
		//glm::vec3 cameraPos = camera.getPosition();

	//shader_.setVar("camPos", camera.getPosition());
	//shader_.setVar("nearPlane", camera.getNearPlane());
	//shader_.setVar("farPlane", camera.getFarPlane());


	//shader_.setVar("cameraMatrix", camera.getMatrix());
	shader_.touch();


	shader_.updateUniform(5, camera.getPosition()); // camPos

	shader_.updateUniform(0, camera.getMatrix()); // cameraMatrix

	shader_.updateUniform(6, camera.getViewMatrix()); // cameraMatrix
	shader_.updateUniform(7, camera.getProjectionMatrix()); // cameraMatrix

	Math::Frustum cameraFrustum = Math::calculateFrustum(camera.getMatrix());
	for (const auto& c : chunks_)
	{
		if (c.second->isVisible(cameraFrustum))
		{

		glm::mat4 model = glm::translate(glm::mat4(1.0f), c.second->getWorldPos());

		// we want shader_.setUniform(1, model); << or something. we don't want to continuously update the value
		// but simply set it.
		// this implys recreating it? no-- yes, no, yes. no.
		shader_.updateUniform(1, model);

		if (c.second->meshBuilt())
		{
			c.second->model_.bind(shader_.pipeline);
			shader_.use(); // DAMN YOU
		}
		}
	}
	shader_.end();
}

void Manager::setBlock(glm::ivec3 pos, const Block& block, bool network)
{
	Chunk* chunk = getChunk(pos);

	/*
	if (chunk == nullptr || block == chunk->getBlock(pos))
		return;

	*/
	if (network) {
		//Network::Instance().registerBlockUpdate({ block.type, pos });

	}

	chunk->setBlock(pos, block);

	// rebuild chunk mesh after modification
	if (chunk->meshBuilt())
		chunk->buildMesh();

	// rebuild surrounding chunks if block was on edge
	for (std::size_t i = 0; i < std::size(Math::surrounding); i++)
	{
		Chunk* adjChunk = getChunk(pos + glm::ivec3(Math::surrounding[i].x, 0.0f, Math::surrounding[i].y));
		if (adjChunk != nullptr && adjChunk != chunk && adjChunk->meshBuilt())
			adjChunk->buildMesh();
	}
}

const Block& Manager::getBlock(glm::ivec3 pos)
{
	Chunk* chunk = getChunk(pos);

	if (chunk == nullptr)
	{
		static Block error = { Block::BLOCK_ERROR };
		return error;
	}

	return chunk->getBlock(pos);
}

std::vector<BlockInfo> Manager::getBlocksInVolume(glm::vec3 pos, glm::vec3 size)
{
	std::vector<BlockInfo> result;

	// AABB bounds
	float xmin = pos.x - size.x / 2.f;
	float xmax = pos.x + size.x / 2.f;
	float ymin = pos.y - size.y / 2.f;
	float ymax = pos.y + size.y / 2.f;
	float zmin = pos.z - size.z / 2.f;
	float zmax = pos.z + size.z / 2.f;

	// AABB collision with blcok grid via indexing
	for (int x = (int)glm::floor(xmin); x <= (int)glm::floor(xmax - (glm::fract(xmax) == 0.0f ? 1.0f : 0.0f)); x++)
	{
		for (int y = (int)glm::floor(ymin); y <= (int)glm::floor(ymax - (glm::fract(ymax) == 0.0f ? 1.0f : 0.0f)); y++)
		{
			for (int z = (int)glm::floor(zmin); z <= (int)glm::floor(zmax - (glm::fract(zmax) == 0.0f ? 1.0f : 0.0f)); z++)
			{
				glm::ivec3 coord = { x, y, z };
				const Block& block = getBlock(coord);
				if (block.type != Block::AIR)
				{
					result.push_back(BlockInfo(coord, block));
				}
			}
		}
	}

	return result;
}

Manager::RaycastResult Manager::raycast(glm::vec3 pos, glm::vec3 dir, float length)
{
	dir = glm::normalize(dir);

	// check for starting inside block
	{
		glm::ivec3 coord = glm::floor(pos);
		Block block = getBlock(coord);
		if (block.type != Block::AIR)
		{
			RaycastResult result;
			result.hit = true;
			result.block = BlockInfo(coord, block);
			result.pos = pos;
			result.normal = Math::vectorToDir(-dir);
			return result;
		}
	}

	while (true)
	{
		// calc distances to block boundaries
		float dists[Math::AXIS_COUNT];
		float min = INFINITY;
		int axis;

		for (int i = 0; i < Math::AXIS_COUNT; i++)
		{
			dists[i] = Math::distToBlock(pos, Math::Axis(i), dir);
			dists[i] *= glm::abs(1.f / dir[i]);
			if (dists[i] < min)
			{
				min = dists[i];
				axis = i;
			}
		}

		// move along ray to next block boundary

		length -= min;

		if (length <= 0)
		{
			RaycastResult result;
			result.hit = false;
			return result;
		}

		pos += dir * min;

		// check for block at block boundary

		glm::ivec3 blockCoord = glm::floor(pos);

		if (dir[axis] < 0.0f)
			blockCoord[axis]--;

		Block block = getBlock(blockCoord);

		if (block.type != Block::AIR)
		{
			RaycastResult result;
			result.hit = true;
			result.block = BlockInfo(blockCoord, block);
			result.pos = pos;
			result.normal = Math::axisToDir(Math::Axis(axis), !(dir[axis] < 0.0f));
			return result;
		}
	}
}

Shader& Manager::getShader()
{
	return shader_;
}

Chunk* Manager::getChunk(glm::ivec3 pos)
{
	// chunks arent const, cast is safe
	return const_cast<Chunk*>(static_cast<const Manager&>(*this).getChunk(pos));
}

const Chunk* Manager::getChunk(glm::ivec3 pos) const
{
	if (pos.y < 0 || pos.y >= World::chunkHeight)
		return nullptr;

	return getChunk(toChunkPos(pos));
}

Chunk* Manager::getChunk(glm::ivec2 chunkCoord)
{
	// chunks arent const, cast is safe
	return const_cast<Chunk*>(static_cast<const Manager&>(*this).getChunk(chunkCoord));
}

const Chunk* Manager::getChunk(glm::ivec2 chunkCoord) const
{
	auto result = chunks_.find(chunkCoord);

	if (result == chunks_.end())
		return nullptr;

	return result->second;
}

glm::ivec2 Manager::toRelativePos(glm::ivec3 pos) const
{
	return { Math::PositiveMod(pos.x, World::chunkSize), Math::PositiveMod(pos.z, World::chunkSize) };
}

glm::ivec2 Manager::toChunkPos(glm::ivec3 pos) const
{
	if (pos.x < 0)
		pos.x -= World::chunkSize - 1;
	if (pos.z < 0)
		pos.z -= World::chunkSize - 1;

	return glm::ivec2(pos.x / (int)World::chunkSize, pos.z / (int)World::chunkSize);
}
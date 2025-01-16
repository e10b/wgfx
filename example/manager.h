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
#include "chunk.h"

#include "database.h"

//class Chunk;
class Camera;

class Manager
{
public:
    typedef std::unordered_map<glm::ivec2, Chunk*> ChunkContainer;


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

	//void drawChunks(const Camera& camera, wgfx::RenderPass& pass);
	void drawChunks(const glm::mat4& cameraMatrix, const Shader& shader, wgfx::RenderPass& pass)
	{
		shader_.updateUniform(0, cameraMatrix); // cameraMatrix
		
		Math::Frustum cameraFrustum = Math::calculateFrustum(cameraMatrix);
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
					//shader_.use(); // DAMN YOU
					pass.draw(shader_.pipeline);
				}
			}
		}
		shader_.end();
	}

	void drawChunksLit(const Camera& camera, wgfx::RenderPass& pass)
	{
		glm::mat4 cameraMatrix = camera.getMatrix();
		
		//camera uniforms
		shader_.updateUniform(5, camera.getPosition()); // camPos
		//near far planes

		// cascades
		drawChunks(cameraMatrix, shader_, pass);
	}

	void setBlock(glm::ivec3 pos, const Block& block, bool network = false);
	const Block& getBlock(glm::ivec3 pos);

	std::vector<BlockInfo> getBlocksInVolume(glm::vec3 pos, glm::vec3 size);
	RaycastResult raycast(glm::vec3 pos, glm::vec3 dir, float length = INFINITY);

	Shader& getShader();
    


	ChunkContainer chunks_;
	Shader shader_;

	//Texture texture_;
    Database db_;

	wgfx::Texture texture_;

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

private:

public:
	Manager(Manager const&) = delete;
	void operator=(Manager const&) = delete;

};
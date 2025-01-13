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

	void drawChunks(const Camera& camera, wgfx::RenderPass& pass);

	void setBlock(glm::ivec3 pos, const Block& block, bool network = false);
	const Block& getBlock(glm::ivec3 pos);

	std::vector<BlockInfo> getBlocksInVolume(glm::vec3 pos, glm::vec3 size);
	RaycastResult raycast(glm::vec3 pos, glm::vec3 dir, float length = INFINITY);

	Shader& getShader();
    /*
    void saveChunks(leveldb::DB* db) {
        //leveldb::DB* db;
        //leveldb::Options options;
        //options.create_if_missing = true;

        //leveldb::Status status = leveldb::DB::Open(options, dbPath, &db);
        //if (!status.ok()) {
            //std::cerr << "Failed to open LevelDB: " << status.ToString() << "\n";
            //return;
        //}

        for (const auto& [coord, chunk] : chunks_) {
            std::ostringstream keyStream;
            keyStream << coord.x << "_" << coord.y;
            std::string key = keyStream.str();

            // Serialize chunk blocks
            std::string value(reinterpret_cast<const char*>(chunk->blocks_.data()),
                sizeof(Block) * chunk->blocks_.size());

            leveldb::Status writeStatus = db->Put(leveldb::WriteOptions(), key, value);
            if (!writeStatus.ok()) {
                std::cerr << "Failed to write chunk to LevelDB: " << writeStatus.ToString() << "\n";
            }
        }

        delete db;
    }

    Chunk* loadChunk(glm::ivec2 coord, leveldb::DB* db) {
        //leveldb::DB* db;
        //leveldb::Options options;

        //leveldb::Status status = leveldb::DB::Open(options, dbPath, &db);
        //if (!status.ok()) {
            //std::cerr << "Failed to open LevelDB: " << status.ToString() << "\n";
            //return nullptr;
        //}

        std::ostringstream keyStream;
        keyStream << coord.x << "_" << coord.y;
        std::string key = keyStream.str();

        std::string value;
        leveldb::Status readStatus = db->Get(leveldb::ReadOptions(), key, &value);
        if (!readStatus.ok()) {
            //std::cerr << "Chunk not found in LevelDB: " << readStatus.ToString() << "\n";
            //delete db;
            return nullptr;
        }

        Chunk* chunk = new Chunk(coord);

        // Deserialize chunk blocks
        if (value.size() != sizeof(Block) * chunk->blocks_.size()) {
            std::cerr << "Chunk data size mismatch\n";
            delete chunk;
            //delete db;
            return nullptr;
        }

        std::memcpy(chunk->blocks_.data(), value.data(), value.size());

        chunks_[coord] = chunk; // Add to the chunks map

        //delete db;
        return chunk;
    }
    */


	ChunkContainer chunks_;
	Shader shader_;
private:

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


public:
	Manager(Manager const&) = delete;
	void operator=(Manager const&) = delete;

};
#include "database.h"

#include "block.h"
#include "chunk.h"
#include "manager.h"

Database::Database(std::string path)
{
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, path, &db_);
	if (!status.ok()) {
		std::cerr << "Unable to open/create LevelDB: " << status.ToString() << std::endl;
	}
}

void Database::saveChunks(Manager& manager)
{
    for (const auto& [coord, chunk] : manager.chunks_) {
        std::ostringstream keyStream;
        keyStream << coord.x << "_" << coord.y;
        std::string key = keyStream.str();

        // Serialize chunk blocks
        std::string value(reinterpret_cast<const char*>(chunk->blocks_.data()),
            sizeof(Block) * chunk->blocks_.size());

        leveldb::Status writeStatus = db_->Put(leveldb::WriteOptions(), key, value);
        if (!writeStatus.ok()) {
            std::cerr << "Failed to write chunk to LevelDB: " << writeStatus.ToString() << "\n";
        }
    }
}

Chunk* Database::loadChunk(glm::ivec2 coord)
{
    std::ostringstream keyStream;
    keyStream << coord.x << "_" << coord.y;
    std::string key = keyStream.str();

    std::string value;
    leveldb::Status readStatus = db_->Get(leveldb::ReadOptions(), key, &value);
    if (!readStatus.ok()) {
        return nullptr;
    }

    Chunk* chunk = new Chunk(coord);

    // Deserialize chunk blocks
    if (value.size() != sizeof(Block) * chunk->blocks_.size()) {
        std::cerr << "Chunk data size mismatch\n";
        delete chunk;
        return nullptr;
    }

    std::memcpy(chunk->blocks_.data(), value.data(), value.size());
    //chunks_[coord] = chunk; // Add to the chunks map
    return chunk;
}
#pragma once

#include <leveldb/include/leveldb/db.h>

#include <iostream>
#include <string>

#include <glm/glm.hpp>

class Chunk;
class Manager;

class Database
{
public:
	Database(std::string path);

	void saveChunks(Manager& manager);

	Chunk* loadChunk(glm::ivec2 coord);

private:
	leveldb::DB* db_;
};
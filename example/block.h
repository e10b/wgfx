#pragma once

#include <utility>

#include "maths.h"

#include <iostream>
#include <array>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

// Textures for each side of a block
struct SideTextureIndicies
{
	SideTextureIndicies(int i) : top(i), side(i), bottom(i) {}
	SideTextureIndicies(int top, int side, int bottom) : top(top), side(side), bottom(bottom) {}

	unsigned top;
	unsigned side;
	unsigned bottom;
};


//Defines info for each block

struct Block
{
	//This matches order in sprite sheet
	enum BlockType : unsigned char
	{
		BLOCK_ERROR = 255,

		AIR = 0,
		GRASS,
		DIRT,
		STONE,
		LOG,
		LEAVES,

		COUNT
	};

	BlockType type;

	Block(BlockType t = AIR) : type(t) {}

	// Serialize the block to a string
	std::string serialize() const {
		return std::to_string(static_cast<unsigned char>(type));
	}

	static Block deserialize(const std::string& data) {
		unsigned char typeValue = static_cast<unsigned char>(std::stoi(data));
		if (typeValue >= COUNT) {
			throw std::runtime_error("Invalid BlockType in deserialization");
		}
		return Block(static_cast<BlockType>(typeValue));
	}

};
typedef std::pair<glm::ivec3, Block> BlockInfo;

// Block-texture data
namespace BlockData
{
	static const SideTextureIndicies sideIndicies[Block::COUNT] =
	{
		0,
		{2, 1, 0},
		0,
		3,
		{5,4,5},
		6,
	};
}
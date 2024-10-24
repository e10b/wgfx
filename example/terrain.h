#pragma once

#include <array>
#include <vector>
#include "maths.h"

#include <unordered_map>
#include <list>
// entry in cache
struct HeightCache
{
	float height; // height zero = disabled
	glm::vec2 pos;
};

// defines method of terrain generation
class Terrain
{
public:
	// get deterministic height value at coord
	int getHeight(glm::vec2 pos);

	// get deterministic tree points in area [start, end)
	std::vector<glm::ivec2> generateTreePoints(glm::ivec2 startCorner, glm::ivec2 endCorner);

	// y, x ,z
	static const int tree[10][7][7];

private:
	static const unsigned cacheCapcaity = 128;

	// cache for height data
	std::array<HeightCache, cacheCapcaity> cache_;
	unsigned cacheSize_ = 0;

	float getNoiseHeight(glm::vec2 pos); // get height of raw noise
	void addToCache(glm::vec2 pos, float height); // add height value to cache
	float getFromCache(glm::vec2 pos); // retrieve height value from cache
};
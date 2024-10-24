#include "terrain.h"
#include "constants.h"

#include <glm/gtc/noise.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp>

#include <random>
#include <iostream>
namespace Gen = World::Generation;

// Hardcoded tree data; in the future, this will go in an external data file
const int Terrain::tree[10][7][7] = {
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 4, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 4, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 4, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 5, 5, 5, 5, 5, 0 },{ 5, 5, 5, 4, 5, 5, 5 },{ 0, 5, 5, 5, 5, 5, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 0, 5, 4, 5, 0, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 5, 5, 5, 5, 5, 0 },{ 0, 5, 5, 4, 5, 5, 0 },{ 0, 5, 5, 5, 5, 5, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 5, 4, 5, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 5, 5, 4, 5, 5, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 5, 5, 5, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
	{{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 5, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0 }},
};

int Terrain::getHeight(glm::vec2 pos)
{
	// get grid corners for interpolation
	glm::vec2 scaled = pos / Gen::terrainInterpGrid;
	glm::vec2 min = glm::floor(scaled) * Gen::terrainInterpGrid;
	glm::vec2 max = glm::ceil(scaled) * Gen::terrainInterpGrid;

	if (min == max)
		return static_cast<int>(getNoiseHeight(pos));

	// interpolate x and y
	if (min.x != max.x && min.y != max.y)
	{
		// get noise at each corner
		float bl = getNoiseHeight(min);
		float tr = getNoiseHeight(max);
		float tl = getNoiseHeight({ min.x, max.y });
		float br = getNoiseHeight({ max.x, min.y });

		// interpolation values
		float tx = (pos.x - min.x) / (max.x - min.x);
		float ty = (pos.y - min.y) / (max.y - min.y);

		// lerp y
		float ml = glm::lerp(bl, tl, ty);
		float mr = glm::lerp(br, tr, ty);

		// lerp x
		return static_cast<int>(glm::lerp(ml, mr, tx));
	}

	// interpolate one dimension
	float minH = getNoiseHeight(min);
	float maxH = getNoiseHeight(max);

	if (min.x == max.x) // interpolate y
		return static_cast<int>(glm::lerp(minH, maxH, (pos.y - min.y) / (max.y - min.y)));
	else // interpolate x
		return static_cast<int>(glm::lerp(minH, maxH, (pos.x - min.x) / (max.x - min.x)));
}

float Terrain::getNoiseHeight(glm::vec2 pos)
{
	// chech cache
	float cache = getFromCache(pos);
	if (cache > 0.0f)
		return cache;

	// calculate layered noise
	float height = ((glm::simplex(pos / float(Gen::heightScale)) + 1) / 2) * Gen::heightWeight * Gen::heightMaxHeight +
		((glm::simplex(pos / float(Gen::detailedScale)) + 1) / 2) * Gen::detailWeight * Gen::detailMaxHeight;

	// apply biome height scalar
	height *= (glm::clamp(
		(glm::simplex(pos / static_cast<float>(Gen::landScale)) + Gen::landMountainBias * 2.0f) * Gen::landTransitionSharpness,
		-1.0f + Gen::landMinMult * 2.0f,
		1.0f
	) + 1.0f) / 2.0f;

	// raise height by min
	height += Gen::minHeight;

	// add and return
	addToCache(pos, height);
	return height;
}

void Terrain::addToCache(glm::vec2 pos, float height)
{
	if (cacheSize_ >= cacheCapcaity)
		cacheSize_ = 0; // reset cache

	// add to cache
	cache_[cacheSize_] = { height, pos };
	cacheSize_++;
}

float Terrain::getFromCache(glm::vec2 pos)
{
	// search from end
	for (int i = cacheSize_ - 1; i >= 0; i--)
	{
		if (cache_[i].pos == pos)
			return cache_[i].height;
	}
	return 0.0f;
}

std::vector<glm::ivec2> Terrain::generateTreePoints(glm::ivec2 startCorner, glm::ivec2 endCorner)
{
	std::vector<glm::ivec2> points;
	std::hash<glm::ivec2> hash;
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	// check each point for if a tree should be generated
	for (int z = startCorner.y; z < endCorner.y; z++)
	{
		for (int x = startCorner.x; x < endCorner.x; x++)
		{
			glm::ivec2 pos = { x, z };
			//std::default_random_engine rng(unsigned(hash(pos)));
			std::mt19937_64 rng(unsigned(hash(pos)));

			// gen tree if using coord as seed passes
			if (dist(rng) <= Gen::treeDensity)
			{
				points.push_back(pos);
			}
		}
	}
	return points;
}
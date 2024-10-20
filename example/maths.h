#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Math
{
	// 3D axes
	enum Axis
	{
		AXIS_X,
		AXIS_Y,
		AXIS_Z,

		AXIS_COUNT
	};

	// 3D directions
	enum Direction
	{
		DIRECTION_LEFT,
		DIRECTION_RIGHT,
		DIRECTION_UP,
		DIRECTION_DOWN,
		DIRECTION_FORWARD,
		DIRECTION_BACKWARD,

		DIRECTION_COUNT
	};

	// Vectors corresponding to direction enum
	const glm::vec3 directionVectors[DIRECTION_COUNT]
	{
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, -1, 0 },
		{ 0, 0, 1 },
		{ 0, 0, -1 },
	};

	// 3x3 vectors of surrounding grid spaces in 2D
	const glm::ivec2 surrounding[] = {
		glm::ivec2(-1.0f,  0.0f),
		glm::ivec2(1.0f,  0.0f),
		glm::ivec2(0.0f, -1.0f),
		glm::ivec2(0.0f,  1.0f),
		glm::ivec2(-1.0f,  1.0f),
		glm::ivec2(1.0f,  1.0f),
		glm::ivec2(1.0f, -1.0f),
		glm::ivec2(-1.0f, -1.0f),
	};

	// 2D corners
	enum Corner
	{
		CORNER_BOTTOM_LEFT,
		CORNER_BOTTOM_RIGHT,
		CORNER_TOP_LEFT,
		CORNER_TOP_RIGHT,

		CORNER_COUNT
	};

	// Conversions
	Direction axisToDir(Axis axis, bool negative);
	Direction vectorToDir(glm::vec3 vec);

	// Modulo that repeats similarly for negative numbers
	float PositiveMod(float val, float mod);
	int PositiveMod(int val, int mod);

	// Normal equation for a plane
	struct Plane
	{
		float a, b, c, d;
	};

	// Bounding planes for frustum
	struct Frustum
	{
		Plane planes[Math::DIRECTION_COUNT];
	};

	// Get frustum from camera matrix
	Frustum calculateFrustum(const glm::mat4& camera);

	// Axis aligned bounding box collision detection
	bool AABBCollision(glm::vec3 pos0, glm::vec3 size0, glm::vec3 pos1, glm::vec3 size1);

	// Distance from position to next block boundary in direction given
	float distToBlock(glm::vec3 pos, Axis axis, glm::vec3 dir);
	float distToBlock(glm::vec3 pos, Axis axis, bool negative);
	float distToBlock(float pos, Axis axis, bool negative);

	// Get UV coordinates from a sprite sheet
	glm::vec2 getUVFromSheet(unsigned sizeX, unsigned sizeY, unsigned index, Corner corner);

}
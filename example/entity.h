#pragma once

#include "maths.h"
#include "block.h"

#include <utility>
#include <vector>

// Defines an object that can move in the world
class Entity
{
public:
	Entity();
	void update(float dt);

	// Getters
	glm::vec3 getPosition() const;
	glm::vec3 getVelocity() const;
	glm::vec3 getSize() const;

	// Movement
	void move(glm::vec3 delta);
	void teleport(glm::vec3 destination);

	void setVelocity(glm::vec3 vel);

	virtual void onCollision(std::pair<std::vector<BlockInfo>, Math::Direction> collision);

private:
	glm::vec3 position_;
	glm::vec3 velocity_;
	glm::vec3 size_;
};
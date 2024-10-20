#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Handles camera data and matrix generation
class Camera
{
public:
	Camera(glm::vec3 position, float fov = 100.0f);

	// Camera getters
	glm::vec3 getPosition() const;
	float getFov() const;
	float getPitch() const;
	float getYaw() const;
	float getNearPlane() const;
	float getFarPlane() const;
	float getAspect() const;

	// Matrix getters
	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getMatrix() const;

	// Vector getters
	glm::vec3 getForward() const;
	glm::vec3 getRight() const;
	glm::vec3 getUp() const;
	glm::vec3 getForwardAligned() const;
	glm::vec3 getUpAligned() const;

	// Movement setters
	void moveForward(float amount);
	void moveRight(float amount);
	void moveUp(float amount);

	// Camera setters
	void setPosition(glm::vec3 pos);
	void setFov(float fov);
	void setPitch(float pitch);
	void setYaw(float yaw);
	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);
	void setAspect(float aspect);

private:
	glm::vec3 pos_;
	float fov_;
	float pitch_ = 0;
	float yaw_ = 0;
	float nearPlane_ = 0.01f; // can cause glitches if too small
	float farPlane_ = 2000.0f;
	float aspect_ = 16.0f / 9.0f;
};
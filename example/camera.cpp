#include "camera.h"
#include "maths.h"

Camera::Camera(glm::vec3 position, float fov)
{
	setPosition(position);
	setFov(fov);
}

glm::vec3 Camera::getPosition() const
{
	return pos_;
}

float Camera::getFov() const
{
	return fov_;
}

float Camera::getPitch() const
{
	return pitch_;
}

float Camera::getYaw() const
{
	return yaw_;
}

float Camera::getNearPlane() const
{
	return nearPlane_;
}

float Camera::getFarPlane() const
{
	return farPlane_;
}

float Camera::getAspect() const
{
	return aspect_;
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(pos_, pos_ + getForward(), getUp());
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::perspective(glm::radians(fov_), aspect_, nearPlane_, farPlane_);
}

glm::mat4 Camera::getMatrix() const
{
	return getProjectionMatrix() * getViewMatrix();
}

glm::vec3 Camera::getForward() const
{
	// Spherical coordinates
	return glm::normalize(glm::vec3(
		-cos(pitch_) * sin(yaw_),
		sin(pitch_),
		-cos(pitch_) * cos(yaw_)
	));
}

glm::vec3 Camera::getRight() const
{
	float angle = yaw_ - glm::half_pi<float>();

	return glm::vec3(
		-sin(angle),
		0,
		-cos(angle)
	);
}

glm::vec3 Camera::getUp() const
{
	return glm::cross(getRight(), getForward());
}

glm::vec3 Camera::getForwardAligned() const
{
	return glm::vec3(
		-sin(yaw_),
		0,
		-cos(yaw_)
	);
}

glm::vec3 Camera::getUpAligned() const
{
	return glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::moveForward(float amount)
{
	pos_ += getForward() * amount;
}

void Camera::moveRight(float amount)
{
	pos_ += getRight() * amount;
}

void Camera::moveUp(float amount)
{
	pos_ += getUp() * amount;
}

void Camera::setPosition(glm::vec3 pos)
{
	pos_ = pos;
}

void Camera::setFov(float fov)
{
	fov_ = glm::clamp(fov, 0.0f, 180.0f);
}

void Camera::setPitch(float pitch)
{
	pitch_ = glm::clamp(pitch, -glm::half_pi<float>(), glm::half_pi<float>());
}

void Camera::setYaw(float yaw)
{
	yaw_ = Math::PositiveMod(yaw, glm::two_pi<float>());
}

void Camera::setNearPlane(float nearPlane)
{
	nearPlane_ = glm::clamp(nearPlane, 0.0f, INFINITY);
}

void Camera::setFarPlane(float farPlane)
{
	farPlane_ = glm::clamp(farPlane, 0.0f, INFINITY);
}

void Camera::setAspect(float aspect)
{
	aspect_ = aspect;
}
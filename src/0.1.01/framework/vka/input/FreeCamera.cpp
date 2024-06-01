#include "FreeCamera.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "../global_state.h"
namespace vka
{
void FreeCamera::keyControl(float deltaTime)
{
	bool* keys = gState.io.keyPressed;
	float velocity = movementSpeed * deltaTime;

	if (keys[GLFW_KEY_W])
	{
		position += front * velocity;
	}

	if (keys[GLFW_KEY_D])
	{
		position += right * velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		position += -right * velocity;
	}

	if (keys[GLFW_KEY_S])
	{
		position += -front * velocity;
	}

	if (keys[GLFW_KEY_SPACE])
	{
		position += -worldUp * velocity;
	}

	if (keys[GLFW_KEY_LEFT_SHIFT])
	{
		position += worldUp * velocity;
	}

	if (keys[GLFW_KEY_P])
	{
		std::cout << "Camera Position: " << glm::to_string(position) << std::endl;
	}

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}
}

void FreeCamera::mouseControl(float deltaTime)
{
	float xChange = turnSpeed * gState.io.mouse.change.x * deltaTime / 0.016;
	float yChange = turnSpeed * gState.io.mouse.change.y * deltaTime / 0.016;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}
	updateRotation();
}

glm::vec3 FreeCamera::getPosition() const
{
	return position;
}

glm::vec3 FreeCamera::getViewDirection() const
{
	return front;
}

glm::mat4 FreeCamera::getViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

void FreeCamera::updateRotation()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front   = glm::normalize(front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}
}        // namespace vka

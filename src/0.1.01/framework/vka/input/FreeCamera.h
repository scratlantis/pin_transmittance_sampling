#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Camera.h"

namespace vka
{
struct FreeCameraCI
{
	glm::vec3 pos;
	glm::vec3 up;
	float     yaw;
	float     pitch;
	float     moveSpeed;
	float     turnSpeed;
};

struct FreeCameraCI_Default : public FreeCameraCI
{
	FreeCameraCI_Default()
	{
		pos = glm::vec3(0.0f, 0.0f, -1.0f);
		up        = glm::vec3(0.0f, 1.0f, 0.0f);
		yaw       = 90.f;
		pitch     = 0.0f;
		moveSpeed = 0.1f;
		turnSpeed = 0.25f;
	}
};

class FreeCamera : public Camera
{
  public:
	FreeCamera(FreeCameraCI ci)
	{
		position     = ci.pos;
		worldUp      = ci.up;
		yaw          = ci.yaw;
		pitch        = ci.pitch;
		movementSpeed = ci.moveSpeed;
		turnSpeed     = ci.turnSpeed;
		updateRotation();
	};
	~FreeCamera(){};
	void      keyControl(float deltaTime);
	void      mouseControl(float deltaTime);
	glm::vec3 getPosition() const;
	glm::vec3 getViewDirection() const;
	glm::mat4 getViewMatrix() const;
  private:
	void updateRotation();
	float   yaw;
	float   pitch;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	float movementSpeed;
	float turnSpeed;
};
}        // namespace vka

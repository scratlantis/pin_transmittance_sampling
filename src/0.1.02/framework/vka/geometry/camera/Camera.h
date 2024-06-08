#pragma once
#include <glm/glm.hpp>
class Camera
{
  public:
	Camera(){};
	~Camera(){};
	virtual void      keyControl(float deltaTime)   = 0;
	virtual void      mouseControl(float deltaTime) = 0;
	virtual glm::vec3 getPosition() const           = 0;
	virtual glm::vec3 getViewDirection() const      = 0;
	virtual glm::mat4 getViewMatrix() const         = 0;
};

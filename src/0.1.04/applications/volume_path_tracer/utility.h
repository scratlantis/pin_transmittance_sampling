#pragma once
#include <vka/vka.h>
#include "config.h"
struct ModelInfo
{
	std::string path;
	glm::vec3   offset;
	float       scale;
	glm::vec3       rotation;

	glm::mat4 getObjToWorldMatrix()
	{
		glm::mat4 objToWorld = glm::mat4(1.0f);
		objToWorld           = glm::translate(objToWorld, offset);
		objToWorld           = glm::rotate(objToWorld, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		objToWorld           = glm::rotate(objToWorld, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		objToWorld           = glm::rotate(objToWorld, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		objToWorld           = glm::scale(objToWorld, glm::vec3(scale));
		return objToWorld;
	};
};

void addFileNamesToEnum(std::string path, std::vector<std::string> &enumVal);

void storeCamState(const FixedCameraState &camState);

static bool mouseInView()
{
	bool inView = true;
	inView      = inView && gState.io.mouse.pos.x > viewDimensions.x * gState.io.extent.width;
	inView      = inView && gState.io.mouse.pos.x < (viewDimensions.x + viewDimensions.width) * gState.io.extent.width;
	inView      = inView && gState.io.mouse.pos.y > viewDimensions.y * gState.io.extent.height;
	inView      = inView && gState.io.mouse.pos.y < (viewDimensions.y + viewDimensions.height) * gState.io.extent.height;
	return inView;
}
static glm::vec2 mouseViewCoord()
{
	glm::vec2 coord;
	coord.x = (gState.io.mouse.pos.x - viewDimensions.x * gState.io.extent.width) / (viewDimensions.width * gState.io.extent.width);
	coord.y = (gState.io.mouse.pos.y - viewDimensions.y * gState.io.extent.height) / (viewDimensions.height * gState.io.extent.height);
	return coord;
}
static glm::vec2 mouseViewChange()
{
	glm::vec2 change;
	change.x = gState.io.mouse.change.x / (viewDimensions.width * gState.io.extent.width);
	change.y = gState.io.mouse.change.y / (viewDimensions.height * gState.io.extent.height);
	return change;
}


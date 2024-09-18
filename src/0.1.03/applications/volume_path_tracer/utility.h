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

FixedCameraState loadCamState();

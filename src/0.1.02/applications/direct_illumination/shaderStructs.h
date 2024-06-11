#pragma once
#include <glm/glm.hpp>

struct Pin
{
	glm::vec2 theta;
	glm::vec2 phi;
};

struct PinData
{
	uint32_t pinIndex;
	uint32_t padding[3];
};
struct PinGridEntry
{
	Pin     pin;
	PinData data;
};

struct Cube
{
	glm::mat4 modelMat;

	glm::mat4 invModelMatrix;
};

struct View
{
	glm::mat4 viewMat;
	glm::mat4 inverseViewMat;
	glm::mat4 projectionMat;
	glm::mat4 inverseProjectionMat;
	glm::mat4 rotationMatrix;

	glm::vec4 camPos;
	uint32_t  width;
	uint32_t  height;
	uint32_t  frameCounter;
	uint32_t  showPins;

	float    pinSelectionCoef;
	float    expMovingAverageCoef;
	uint32_t secondaryWidth;
	uint32_t secondaryHeight;

	glm::mat4 secondaryProjectionMat;
	glm::mat4 secondaryInverseProjectionMat;
	glm::vec4 probe;

	glm::mat4 fogModelMatrix;
	glm::mat4 fogInvModelMatrix;

	float    secRayLength;
	float    positionalJitter;
	float    angularJitter;
	uint32_t useEnvMap;

	Cube cube;
};

struct Gaussian
{
	glm::vec3 mean;
	float     variance;
};
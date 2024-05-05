#pragma once
#include <glm/glm.hpp>
#define GAUSSIAN_COUNT 10
#define PIN_GRID_SIZE 20
#define PINS_PER_GRID_CELL 20
#define PI 3.14159265359
#define PIN_COUNT PIN_GRID_SIZE *PIN_GRID_SIZE *PIN_GRID_SIZE *PINS_PER_GRID_CELL

struct Pin
{
	glm::vec2 theta;
	glm::vec2 phi;
};

struct Cube
{
	glm::mat4 modelMat;

	glm::mat4 invModelMatrix;
};

struct PerFrameConstants
{
	glm::vec4 camPos;

	glm::mat4 viewMat;

	glm::mat4 inverseViewMat;

	glm::mat4 projectionMat;

	glm::mat4 inverseProjectionMat;

	uint32_t width;
	uint32_t height;
	uint32_t frameCounter;
	uint32_t mousePosX;

	uint32_t mousePosY;
	uint32_t usePins;
	uint32_t placeholder3;
	uint32_t placeholder4;

	Cube cube;
};
struct Gaussian
{
	glm::vec3 mean;
	float     variance;
};
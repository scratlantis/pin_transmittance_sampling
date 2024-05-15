#pragma once
#include <glm/glm.hpp>
#define GAUSSIAN_COUNT 10
#define GAUSSIAN_MARGIN 0.2f
#define PIN_GRID_SIZE 20
#define PINS_PER_GRID_CELL 20
#define PI 3.14159265359
#define PIN_COUNT PIN_GRID_SIZE *PIN_GRID_SIZE *PIN_GRID_SIZE *PINS_PER_GRID_CELL
#include <framework/vka/core/utility/misc.h>
#include <framework/vka/global_state.h>

GVar gvar_use_pins{"use pins", true, GVAR_BOOL, GVAR_APPLICATION};

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
	Pin pin;
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
	uint32_t width;
	uint32_t height;
	uint32_t frameCounter;
	uint32_t usePins;
	Cube      cube;
	void update(uint32_t& cnt, Camera& camera)
	{
		width				 = gState.io.extent.width;
		height               = gState.io.extent.height;
		frameCounter         = cnt++;
		camPos               = glm::vec4(camera.get_camera_position(), 1.0);
		viewMat              = camera.calculate_viewmatrix();
		//viewMat              = glm::mat4(1.0);
		//viewMat[3]           = glm::vec4(0.0, 0.0, 0.0, 1.0);
		inverseViewMat       = glm::inverse(viewMat);
		projectionMat        = glm::perspective(glm::radians(60.0f), (float) gState.io.extent.width / (float) gState.io.extent.height, 0.1f, 500.0f);
		//projectionMat = glm::mat4(1.0);
		inverseProjectionMat = glm::inverse(projectionMat);
		cube             = Cube{glm::mat4(1.0), glm::mat4(1.0)};
		usePins = gvar_use_pins.val.bool32();
	}
};


struct Gaussian
{
	glm::vec3 mean;
	float     variance;

};
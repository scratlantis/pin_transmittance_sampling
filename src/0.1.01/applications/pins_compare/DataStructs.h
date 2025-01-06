#pragma once
#include <glm/glm.hpp>
#define GAUSSIAN_COUNT 10
#define GAUSSIAN_MARGIN 0.2f
#define PIN_GRID_SIZE 20
#define PINS_PER_GRID_CELL 20
#define PI 3.14159265359
#define PIN_COUNT_SQRT 100
#define PIN_COUNT PIN_COUNT_SQRT *PIN_COUNT_SQRT

#define GAUSS_FILTER_RADIUS 4


#include <vka/core/utility/misc.h>
#include <vka/global_state.h>


const std::string shaderPathPrefix = std::string(APP_SRC_DIR) + "/shaders/";

GVar gvar_use_pins{"show pins", 0, GVAR_ENUM, GVAR_APPLICATION, {"None", "Activ", "All"}};
GVar gvar_pin_selection_coef{"pin selection coef", 1.0f, GVAR_UNORM, GVAR_APPLICATION};
GVar gvar_use_exp_moving_average{"use exponential moving average", false, GVAR_BOOL, GVAR_APPLICATION};
GVar gvar_use_gaus_blur{"use gauss blur", false, GVAR_BOOL, GVAR_APPLICATION};
GVar gvar_exp_moving_average_coef{"exp moving average coef", 0.0f, GVAR_UNORM, GVAR_APPLICATION};

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
	uint32_t padding[2];

	Cube cube;
	void update(uint32_t &cnt, Camera &camera)
	{
		width        = gState.io.extent.width;
		height       = gState.io.extent.height;
		frameCounter = cnt++;
		camPos       = glm::vec4(camera.getPosition(), 1.0);
		viewMat      = camera.getViewMatrix();
		// viewMat              = glm::mat4(1.0);
		// viewMat[3]           = glm::vec4(0.0, 0.0, 0.0, 1.0);
		inverseViewMat = glm::inverse(viewMat);
		projectionMat  = glm::perspective(glm::radians(60.0f), (float) gState.io.extent.width / (float) gState.io.extent.height, 0.1f, 500.0f);
		// projectionMat = glm::mat4(1.0);
		inverseProjectionMat = glm::inverse(projectionMat);
		cube                 = Cube{glm::mat4(1.0), glm::mat4(1.0)};
		showPins              = gvar_use_pins.val.v_int;
		pinSelectionCoef     = gvar_pin_selection_coef.val.v_float;
		expMovingAverageCoef = gvar_exp_moving_average_coef.val.v_float;
	}
};

struct Gaussian
{
	glm::vec3 mean;
	float     variance;
};
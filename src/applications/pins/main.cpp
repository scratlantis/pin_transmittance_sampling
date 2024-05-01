#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "stdio.h"
#include <framework/vka/global_state.h>
#include <framework/vka/initializers/misc.h>
#include <framework/vka/setup/GlfwWindow.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <framework/vka/combined_resources/AccelerationStructur.h>
#include <framework/vka/combined_resources/Buffer.h>
#include <framework/vka/combined_resources/CmdBuffer.h>
#include <framework/vka/combined_resources/Image.h>
#include <framework/vka/input/Camera.h>
#include <framework/vka/resources/ComputePipeline.h>
#include <framework/vka/resources/DescriptorSetLayout.h>
#include <framework/vka/resources/Shader.h>
#include <random>

using namespace vka;
#ifndef SHADER_DIR
#	define SHADER_DIR ""
#endif        // !TARGET_NAME     // !TARGET_NAME

using namespace vka;
#ifndef APP_NAME
#	define APP_NAME ""
#endif        // !TARGET_NAME     // !TARGET_NAME

GVar                gvar_test_button{"test_button", false, GVAR_BOOL, GVAR_APPLICATION};
std::vector<GVar *> gVars{
    &gvar_test_button};
AppState          gState;
const std::string gShaderPath = SHADER_DIR;

#define UPLOAD_IDLE(data, size, buffer)                                                                       \
	CmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT); \
	cmdBuf.uploadData(data, size, buffer);                                                                    \
	commitCmdBuffers({cmdBuf}, &gState.frame->stack, gState.device.universalQueues[0]);                       \
	vkDeviceWaitIdle(gState.device.logical);

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
	glm::mat4 centerSize;
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
	uint32_t invertColors;
	uint32_t placeholder3;
	uint32_t placeholder4;

	Cube cube;
};
struct Gaussian
{
	glm::vec3 mean;
	float     variance;
};

int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 400, 400);
	Window       *window   = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Camera initialization
	Camera camera = Camera(CameraCI_Default());
	// Resource Creation
	FramebufferImage                      offscreenImage = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, gState.io.format, gState.io.extent);
	Buffer                                ubo            = BufferVma(&gState.heap, sizeof(PerFrameConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Buffer                                gaussiansBuf   = BufferVma(&gState.heap, sizeof(Gaussian) * GAUSSIAN_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	std::mt19937                          gen32(42);
	std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);

	// Init gaussians:
	std::vector<Gaussian> gaussiansData(GAUSSIAN_COUNT);
	float                 coef = 0.3;
	for (size_t i = 0; i < GAUSSIAN_COUNT; i++)
	{
		gaussiansData[i].mean.x   = (1.0 - coef) / 2.0 + coef * unormDistribution(gen32);
		gaussiansData[i].mean.y   = (1.0 - coef) / 2.0 + coef * unormDistribution(gen32);
		gaussiansData[i].mean.z   = (1.0 - coef) / 2.0 + coef * unormDistribution(gen32);
		gaussiansData[i].variance = 0.5 * coef * unormDistribution(gen32);
	}
	std::vector<Pin> pinGrid(PIN_GRID_SIZE * PIN_GRID_SIZE * PIN_GRID_SIZE * PINS_PER_GRID_CELL);
	std::vector<Pin> pins(PIN_COUNT);
	Buffer           pinBuf              = BufferVma(&gState.heap, sizeof(Pin) * pins.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Buffer           pinTransmittanceBuf = BufferVma(&gState.heap, sizeof(float) * pins.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	Buffer           pinGridBuf          = BufferVma(&gState.heap, sizeof(Pin) * pinGrid.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	Buffer           pinGridIdBuf        = BufferVma(&gState.heap, sizeof(uint32_t) * pinGrid.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	// Init pins:
	for (size_t i = 0; i < pins.size(); i++)
	{
		pins[i].theta.x = 2.0 * PI * unormDistribution(gen32);
		pins[i].theta.y = 2.0 * PI * unormDistribution(gen32);
		pins[i].phi.x   = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
		pins[i].phi.y   = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
	}
	ComputeCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	// Upload data
	cmdBuf.uploadData(gaussiansData.data(), sizeof(Gaussian) * GAUSSIAN_COUNT, gaussiansBuf);
	cmdBuf.uploadData(pins.data(), sizeof(Pin) * pins.size(), pinBuf);
	cmdBuf.barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	// Calc pin transmittance in shader:
	if (1)
	{
		glm::uvec3           workGroupSize  = {128, 1, 1};
		glm::uvec3           resolution     = {pins.size(), 1, 1};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = "pins_eval_transmittance.comp";
		computeState.shaderDef.args.push_back({"GAUSSIAN_COUNT", std::to_string(GAUSSIAN_COUNT)});
		computeState.shaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
		computeState.specialisationEntrySizes           = glm3VectorSizes();
		computeState.specializationData                 = getByteVector(workGroupSize);
		ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
		cmdBuf.bindPipeline(computePipeline);
		cmdBuf.pushDescriptors(0, gaussiansBuf, pinBuf, pinTransmittanceBuf);
		cmdBuf.dispatch(workGroupCount);
	}
	// Generate pin grid in shader:
	if (1)
	{
		glm::uvec3           workGroupSize  = {8, 8, 8};
		glm::uvec3           resolution     = {PIN_GRID_SIZE, PIN_GRID_SIZE, PIN_GRID_SIZE};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = "pins_grid_gen.comp";
		computeState.shaderDef.args.push_back({"PIN_GRID_SIZE", std::to_string(PIN_GRID_SIZE)});
		computeState.shaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		computeState.shaderDef.args.push_back({"PINS_PER_GRID_CELL", std::to_string(PINS_PER_GRID_CELL)});
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
		computeState.specialisationEntrySizes           = glm3VectorSizes();
		computeState.specializationData                 = getByteVector(workGroupSize);
		ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
		cmdBuf.bindPipeline(computePipeline);
		cmdBuf.pushDescriptors(0, pinBuf, pinGridBuf, pinGridIdBuf);
		cmdBuf.dispatch(workGroupCount);
	}
	commitCmdBuffers({cmdBuf}, &gState.frame->stack, gState.device.universalQueues[0]);
	vkDeviceWaitIdle(gState.device.logical);

	uint32_t cnt = 0;
	while (!gState.io.shouldTerminate())
	{
		// Hot Reload
		if (gState.io.keyEvent[GLFW_KEY_R] && gState.io.keyPressed[GLFW_KEY_R])
		{
			vkDeviceWaitIdle(gState.device.logical);
			gState.cache.clear();
		}
		// Update Camera
		camera.key_control(gState.io.keyPressed, 0.016);

		if (gState.io.mouse.leftPressed)
		{
			camera.mouse_control(gState.io.mouse.change.x, gState.io.mouse.change.y);
		}
		// Update per frame constants
		PerFrameConstants pfc{};
		pfc.width                = gState.io.extent.width;
		pfc.height               = gState.io.extent.height;
		pfc.invertColors         = gvar_test_button.val.bool32();
		pfc.frameCounter         = cnt++;
		pfc.mousePosX            = gState.io.mouse.pos.x;
		pfc.mousePosY            = gState.io.mouse.pos.y;
		pfc.camPos               = glm::vec4(camera.get_camera_position(), 1.0);
		pfc.viewMat              = camera.calculate_viewmatrix();
		pfc.viewMat[3]           = glm::vec4(0.0, 0.0, 0.0, 1.0);
		pfc.inverseViewMat       = glm::inverse(pfc.viewMat);
		pfc.projectionMat        = glm::perspective(glm::radians(60.0f), (float) gState.io.extent.width / (float) gState.io.extent.height, 1.0f, 500.0f);
		pfc.inverseProjectionMat = glm::inverse(pfc.projectionMat);
		pfc.cube                 = Cube{glm::mat4(1.0), glm::mat4(1.0)};
		// Pipeline Creation
		glm::uvec3           workGroupSize  = {16, 16, 1};
		glm::uvec3           resolution     = {gState.io.extent.width, gState.io.extent.height, 1};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = "pins_render.comp";
		computeState.shaderDef.args.push_back({"GAUSSIAN_COUNT", std::to_string(GAUSSIAN_COUNT)});
		computeState.shaderDef.args.push_back({"PIN_GRID_SIZE", std::to_string(PIN_GRID_SIZE)});
		computeState.shaderDef.args.push_back({"PIN_COUNT", std::to_string(PIN_COUNT)});
		computeState.shaderDef.args.push_back({"PINS_PER_GRID_CELL", std::to_string(PINS_PER_GRID_CELL)});
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
		layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		layoutDefinition.flags                          = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
		computeState.specialisationEntrySizes           = glm3VectorSizes();
		computeState.specializationData                 = getByteVector(workGroupSize);
		ComputePipeline computePipeline                 = ComputePipeline(&gState.cache, computeState);
		// Record commands
		ComputeCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		offscreenImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
		cmdBuf.transitionLayout(offscreenImage, VK_IMAGE_LAYOUT_GENERAL);
		cmdBuf.bindPipeline(computePipeline);
		cmdBuf.uploadData(&pfc, sizeof(pfc), ubo);
		cmdBuf.pushDescriptors(0, ubo, (Image) offscreenImage, pinTransmittanceBuf, pinGridBuf, pinGridIdBuf, gaussiansBuf);
		cmdBuf.dispatch(workGroupCount);
		cmdBuf.copyToSwapchain(offscreenImage);
		// Submit commands and present
		gState.swapBuffers({cmdBuf});
	}
	// Cleanup
	gState.destroy();
	delete window;
}
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "stdio.h"
#include <vka/global_state.h>
#include <vka/initializers/misc.h>
#include <vka/setup/GlfwWindow.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vka/combined_resources/AccelerationStructur.h>
#include <vka/combined_resources/Buffer.h>
#include <vka/combined_resources/CmdBuffer.h>
#include <vka/combined_resources/Image.h>
#include <vka/input/Camera.h>
#include <vka/resources/ComputePipeline.h>
#include <vka/resources/DescriptorSetLayout.h>
#include <vka/resources/Shader.h>

using namespace vka;
#ifndef SHADER_DIR
#	define SHADER_DIR ""
#endif        // !TARGET_NAME     // !TARGET_NAME

using namespace vka;
#ifndef APP_NAME
#	define APP_NAME ""
#endif        // !TARGET_NAME     // !TARGET_NAME

AppState          gState;
const std::string gShaderPath = SHADER_DIR;

GVar                gvar_test_button{"test_button", false, GVAR_BOOL, GVAR_APPLICATION};
std::vector<GVar *> gVars{
    &gvar_test_button};

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



int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 400, 400);
	Window       *window   = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Camera initialization
	FreeCamera camera = FreeCamera(CameraCI_Default());
	// Resource Creation
	FramebufferImage offscreenImage = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, gState.io.format, gState.io.extent);
	Buffer           ubo            = BufferVma(&gState.heap, sizeof(PerFrameConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	uint32_t         cnt            = 0;
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
		pfc.width        = gState.io.extent.width;
		pfc.height       = gState.io.extent.height;
		pfc.invertColors = gvar_test_button.val.bool32();
		pfc.frameCounter = cnt++;
		pfc.mousePosX    = gState.io.mouse.pos.x;
		pfc.mousePosY    = gState.io.mouse.pos.y;
		pfc.camPos       = glm::vec4(camera.get_camera_position(), 1.0);

		if (cnt % 200 == 0)
		{
			//printVka("Camera Position %.3f, %.3f, %.3f\n", pfc.camPos.x, pfc.camPos.y, pfc.camPos.z);
			printVka("Camera Position %.3f\n", pfc.camPos.x);
			//printVka("Camera Dir %.3f, %.3f, %.3f\n", camera.get_front_axis());
		}

		pfc.viewMat              = camera.calculate_viewmatrix();
		pfc.viewMat[3]           = glm::vec4(0.0, 0.0, 0.0, 1.0);
		pfc.inverseViewMat       = glm::inverse(pfc.viewMat);
		pfc.projectionMat        = glm::perspective(glm::radians(60.0f), (float) gState.io.extent.width / (float) gState.io.extent.height, 1.0f, 500.0f);
		pfc.inverseProjectionMat = glm::inverse(pfc.projectionMat);

		pfc.cube = Cube{glm::mat4(1.0), glm::mat4(1.0)};

		// Pipeline Creation
		glm::uvec3           workGroupSize  = {16, 16, 1};
		glm::uvec3           resolution     = {gState.io.extent.width, gState.io.extent.height, 1};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = APP_NAME;
		computeState.shaderDef.name.append(".comp");
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
		layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
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
		cmdBuf.pushDescriptors(0, ubo, (Image) offscreenImage);
		cmdBuf.dispatch(workGroupCount);
		cmdBuf.copyToSwapchain(offscreenImage);
		// Submit commands and present
		gState.swapBuffers({cmdBuf});
	}
	// Cleanup
	gState.destroy();
	delete window;
}
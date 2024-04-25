#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "stdio.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <framework/vka/global_state.h>
#include <framework/vka/initializers/misc.h>
#include <framework/vka/setup/GlfwWindow.h>

#include <framework/vka/combined_resources/Image.h>
#include <framework/vka/combined_resources/Buffer.h>
#include <framework/vka/combined_resources/CmdBuffer.h>
#include <framework/vka/combined_resources/AccelerationStructur.h>
#include <framework/vka/resources/ComputePipeline.h>
#include <framework/vka/resources/Shader.h>
#include <framework/vka/resources/DescriptorSetLayout.h>

using namespace vka;
#ifndef SHADER_DIR
#	define SHADER_DIR ""
#endif        // !TARGET_NAME     // !TARGET_NAME

vka::AppState gState;

const std::string gShaderPath = SHADER_DIR;

struct PerFrameConstants
{
	glm::mat4 viewMat;
	glm::mat4 inverseViewMat;
	glm::mat4 projectionMat;
	glm::mat4 inverseProjectionMat;

	uint32_t width;
	uint32_t height;
	uint32_t frameCounter;
	uint32_t mousePosX;

	uint32_t mousePosY;
	uint32_t placeholder2;
	uint32_t placeholder3;
	uint32_t placeholder4;
};

int main()
{
	// Global State Initialization
	DeviceCI deviceCI = D3VKPTDeviceCI("test_application");
	IOControlerCI ioCI     = DefaultIOControlerCI("test_window", 400,400);
	Window* window = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Resource Creation
	FramebufferImage offscreenImage = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, gState.io.format, gState.io.extent);
	FramebufferImage inputImage     = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_FORMAT_R16G16B16A16_UNORM, gState.io.extent);
	FramebufferImage outputImage     = FramebufferImage(&gState.heap, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_FORMAT_R16G16B16A16_UNORM, gState.io.extent);
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
		// Pipeline Creation
		glm::uvec3           workGroupSize  = {1, 1, 1};
		glm::uvec3           resolution     = {gState.io.extent.width, gState.io.extent.height, 1};
		glm::uvec3           workGroupCount = getWorkGroupCount(workGroupSize, resolution);
		ComputePipelineState computeState{};
		computeState.shaderDef.name = "test_shader.comp";
		DescriptorSetLayoutDefinition layoutDefinition{};
		layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
		layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
		layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
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
		inputImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
		cmdBuf.transitionLayout(inputImage, VK_IMAGE_LAYOUT_GENERAL);
		outputImage.update(&gState.heap, &gState.frame->stack, gState.io.extent);
		cmdBuf.transitionLayout(outputImage, VK_IMAGE_LAYOUT_GENERAL);

		cmdBuf.bindPipeline(computePipeline);
		PerFrameConstants pfc{};
		pfc.width = gState.io.extent.width;
		pfc.height = gState.io.extent.height;
		pfc.frameCounter = cnt++;
		pfc.mousePosX    = gState.io.mouse.pos.x;
		pfc.mousePosY    = gState.io.mouse.pos.y;
		cmdBuf.uploadData(&pfc, sizeof(pfc), ubo);
		cmdBuf.pushDescriptors(0, ubo, (Image) offscreenImage, (Image) inputImage, (Image) outputImage);
		cmdBuf.dispatch(workGroupCount);
		cmdBuf.copyToSwapchain(offscreenImage);
		cmdBuf.copyImage(outputImage, inputImage);
		// Submit commands and present
		gState.swapBuffers({cmdBuf});
	}
	// Cleanup
	gState.destroy();
	delete window;
}
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
	uint32_t placeholder1;
	uint32_t placeholder2;
};

int main()
{
	// Global State Initialization
	DeviceCI deviceCI = D3VKPTDeviceCI("test_application");
	IOControlerCI ioCI     = DefaultIOControlerCI("test_window", 400,400);
	Window* window = new vka::GlfwWindow();
	gState.init(deviceCI, ioCI, window);
	// Resource Creation
	VkImageCreateInfo imageCI = vka::ImageCreateInfo_Swapchain(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	Image offscreenImage = ImageVma(&gState.heap, imageCI, VMA_MEMORY_USAGE_GPU_ONLY);
	offscreenImage.createImageView(&gState.heap, ImageViewCreateInfo_Swapchain(offscreenImage.img, offscreenImage.format));
	// Pipeline Creation
	glm::uvec3 workGroupSize = {1, 1, 1};
	glm::uvec3 resolution    = {gState.io.extent.width, gState.io.extent.height, 1};
	glm::uvec3 workGroupCount = getWorkGroupCount(workGroupSize, resolution);
	ComputePipelineState computeState{};
	computeState.shaderDef.name = "test_shader.comp";
	DescriptorSetLayoutDefinition layoutDefinition{};
	layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
	layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
	layoutDefinition.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
	computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
	computeState.specialisationEntrySizes           = glm3VectorSizes();
	computeState.specializationData                 = getByteVector(workGroupSize);
	ComputePipeline computePipeline = ComputePipeline(&gState.cache, computeState);
	while (!gState.io.shouldTerminate())
	{
		// Record commands
		ComputeCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		cmdBuf.transitionLayout(offscreenImage, VK_IMAGE_LAYOUT_GENERAL);
		cmdBuf.bindPipeline(computePipeline);
		PerFrameConstants pfc{};
		pfc.width = gState.io.extent.width;
		pfc.height = gState.io.extent.height;
		cmdBuf.pushDescriptors(0, pfc, offscreenImage);
		cmdBuf.dispatch(workGroupCount);
		cmdBuf.copyToSwapchain(offscreenImage);
		// Submit commands and present
		gState.swapBuffers({cmdBuf});
	}
	// Cleanup
	gState.destroy();
	delete window;
}
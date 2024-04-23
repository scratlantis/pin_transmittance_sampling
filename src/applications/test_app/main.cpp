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
	DeviceCI deviceCI = D3VKPTDeviceCI("test_application");
	IOControlerCI ioCI     = DefaultIOControlerCI("test_window", 400,400);
	Window* window = new vka::GlfwWindow();

	gState.init(deviceCI, ioCI, window);

	glm::uvec3 workGroupSize = {16, 16, 1};
	glm::uvec3 resolution    = {gState.io.extent.width, gState.io.extent.height, 1};
	glm::uvec3 workGroupCount = getWorkGroupCount(workGroupSize, workGroupCount);


	VkImageCreateInfo imageCI = vka::ImageCreateInfo_Swapchain(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	Image offscreenImage = ImageVma(&gState.heap, imageCI, VMA_MEMORY_USAGE_GPU_ONLY);

	ComputePipelineState computeState{};
	computeState.shaderDef.name = "test_shader.comp";
	DescriptorSetLayoutDefinition layoutDefinition{};
	layoutDefinition.addStorageImage(VK_SHADER_STAGE_COMPUTE_BIT);
	layoutDefinition.addUniformBuffer(VK_SHADER_STAGE_COMPUTE_BIT);
	computeState.pipelineLayoutDef.descSetLayoutDef = {layoutDefinition};
	computeState.specialisationEntrySizes           = glm3VectorSizes();
	computeState.specializationData                 = getByteVector(workGroupSize);
	ComputePipeline computePipeline = ComputePipeline(&gState.heap, computeState);
	VkPipeline vkPipeline = computePipeline.getHandle();
	while (!gState.io.shouldTerminate())
	{
		ComputeCmdBuffer cmdBuf = UniversalCmdBuffer(&gState.frame->stack, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		cmdBuf.bindPipeline(computePipeline);
		PerFrameConstants pfc{};
		cmdBuf.pushDescriptors(0, pfc, offscreenImage);
		cmdBuf.dispatch(workGroupCount);
		gState.swapBuffers({cmdBuf});
	}
	gState.destroy();
	delete window;
}
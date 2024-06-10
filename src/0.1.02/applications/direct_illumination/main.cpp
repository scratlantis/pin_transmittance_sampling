#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <vka/vka.h>
#include <vka/state_objects/GlfwWindow.h>
#include <vka/render/common.h>
#include <vka/geometry/common.h>
#include <vka/render/tools/FramebufferCache.h>
#include "config.h"

using namespace vka;
AppState gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gResourceBaseDir = RESOURCE_BASE_DIR;
std::vector<GVar *> gVars = {};
int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow    window   = GlfwWindow();
	gState.init(deviceCI, ioCI, &window);
	FixedCamera camera = FixedCamera(FixedCameraCI_Default());
	GvarGui     gui    = GvarGui();

	// Init:
	VkaImage         swapchainImage   = vkaGetSwapchainImage();
	FramebufferCache framebufferCache = FramebufferCache();
	ResourcePool     heap             = ResourcePool();

	VkaImage offscreenImage = vkaCreateSwapchainAttachment(&heap, gState.io.format,
	                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	VkaImage depthImage     = vkaCreateSwapchainAttachment(&heap, VK_FORMAT_D32_SFLOAT,
	                                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	VkaImage lineColorImg   = vkaCreateSwapchainAttachment(&heap, gState.io.format,
	                                                       VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	VkaImage linePosImg     = vkaCreateSwapchainAttachment(&heap, VK_FORMAT_R32G32B32A32_SFLOAT,
	                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	VkaImage pinIdImage     = vkaCreateSwapchainAttachment(&heap, VK_FORMAT_R32_UINT,
	                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	VkaBuffer viewBuf             = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	VkaBuffer gaussianBuf         = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	VkaBuffer pinBuf              = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	VkaBuffer pinDirectionsBuffer = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	VkaBuffer pinGridBuf          = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	VkaBuffer pinTransmittanceBuf = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	VkaBuffer pinVertexBuffer     = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	VkaBuffer pinIndexBuffer      = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	VkaBuffer pinUsedBuffer       = vkaCreateBuffer(&heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);


	// Vertex/Index Data
	// Geometry(Vertex/Index Data)
	// Material(Buffers)
	// Model(Geometry, Material)
	// Instance(Model, Transform)
	// DrawCall(Instances)
	


	// VertexBuffer
	// IndexBuffer
	// PipelineConfig
	// RenderTargetList
	// InstanceBufferList
	// DrawCall(DrawCallCI)




	// create buffers
	// model cache
	// render model

	// Upload:
	// upload buffers
	// upload images


	// Main loop:
	while (!gState.io.shouldTerminate())
	{
		if (gState.io.keyEvent[GLFW_KEY_R] && gState.io.keyPressed[GLFW_KEY_R])
		{
			vkDeviceWaitIdle(gState.device.logical);
			gState.cache->clearShaders();
		}

		// StartCmdBuf
		// Render
		// SwapBuffers
	}
	// Cleanup
	vkDeviceWaitIdle(gState.device.logical);
	gState.destroy();
}
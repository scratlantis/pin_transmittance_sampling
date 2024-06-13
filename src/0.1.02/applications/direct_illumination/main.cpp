#include <vka/vka.h>
#include "config.h"
#include "app_data.h"
#include "shaderStructs.h"

using namespace vka;
AppState gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string   gResourceBaseDir = RESOURCE_BASE_DIR;
// clang-format off
std::vector<GVar *> gVars =
{
        &gvar_use_pins
};
// clang-format on
int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow    window   = GlfwWindow();
	gState.init(deviceCI, ioCI, &window);
	GvarGui     gui    = GvarGui();
	// Init:
	VkaImage         swapchainImage   = vkaGetSwapchainImage();
	FramebufferCache framebufferCache = FramebufferCache();
	ResourcePool     heap             = ResourcePool();
	ModelCache       modelCache       = ModelCache(&heap, gResourceBaseDir + "/models");
	// TextureCache textureCache = TextureCache(&heap, gResourceBaseDir + "/textures");
	AppData appData = AppData();

	// Create Images
	VkaImage offscreenImage = vkaCreateSwapchainAttachment(gState.io.format,
	                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	VkaImage depthImage     = vkaCreateSwapchainAttachment(VK_FORMAT_D32_SFLOAT,
	                                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	VkaImage lineColorImg   = vkaCreateSwapchainAttachment(gState.io.format,
	                                                       VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	VkaImage linePosImg     = vkaCreateSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
	                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	VkaImage pinIdImage     = vkaCreateSwapchainAttachment(VK_FORMAT_R32_UINT,
	                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);


	RenderPassDefinition guiRenderPassDef = RenderPassDefinition();
	setDefaults(guiRenderPassDef, 1 , 1);
	addColorAttachment(guiRenderPassDef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, gState.io.format, true);
	// Todo: configure gui render pass



	VkRenderPass         guiRenderPass    = gState.cache->fetch(guiRenderPassDef);
	gui.create(guiRenderPass, 0);
	

	
	AppConfig appConfig = AppConfig();
	VkaCommandBuffer cmdBuf = vkaCreateCommandBuffer(gState.frame->stack);
	gState.io.swapchainAttachmentPool->refreshImages(cmdBuf);
	appData.update(cmdBuf, appConfig);
	gui.upload(cmdBuf);
	vkaExecuteImmediat(cmdBuf);
	gui.freeStaging();


	// Main loop:
	while (!gState.io.shouldTerminate())
	{
		gui.newFrame();
		// Reload shaders
		if (gState.io.keyEvent[GLFW_KEY_R] && gState.io.keyPressed[GLFW_KEY_R])
		{
			vkDeviceWaitIdle(gState.device.logical);
			gState.cache->clearShaders();
		}
		// Resize window
		if (gState.io.swapchainRecreated())
		{
			framebufferCache.clear();
		}

		VkaCommandBuffer cmdBuf = vkaCreateCommandBuffer(gState.frame->stack);
		if (0) // Gui button
		{
			appData.update(cmdBuf, appConfig);
		}

		// Update view
		{
			// ...
		}

		// Render
		{
			DrawCmd drawCmd{};
			drawCmd.model = modelCache.fetch(cmdBuf, "cube/cube.obj", sizeof(PosVertex), PosVertex::parse);
			vka::setDefaults(drawCmd.pipelineDef, RasterizationPipelineDefaultValues(), 1, 2);
			drawCmd.attachments = {depthImage, offscreenImage};
			drawCmd.framebuffer = framebufferCache.fetch(gState.cache->fetch(drawCmd.pipelineDef.renderPassDefinition), drawCmd.attachments);
			drawCmd.descriptors = {appData.viewBuf, appData.gaussianBuf, &appData.envMapSamplerDef, lineColorImg, linePosImg};
			drawCmd.clearValues = {VK_CLEAR_COLOR_MAX_DEPTH, VK_CLEAR_COLOR_WHITE};
			drawCmd.instanceBuffers = {appData.gaussianFogCubeTransformBuf};
			drawCmd.instanceCount = 1;


			// ...


			// Render gui
			{
				vkaCmdStartRenderPass(cmdBuf, guiRenderPass, framebufferCache.fetch(guiRenderPass, {vkaGetSwapchainImage()}), {VK_CLEAR_COLOR_BLACK});
				gui.render(cmdBuf);
				vkaCmdEndRenderPass(cmdBuf);
			}
		}

		vkaSwapBuffers({cmdBuf});
	}
	// Cleanup
	vkDeviceWaitIdle(gState.device.logical);
	framebufferCache.clear();
	heap.clear();
	gui.destroy();
	gState.destroy();
}
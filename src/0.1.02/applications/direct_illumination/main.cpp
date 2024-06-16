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
        //&gvar_use_pins
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
	std::string shaderDir = std::string(APP_SRC_DIR) + "/shaders/";
	appData.init(&heap);

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
	setDefaults(guiRenderPassDef);
	addColorAttachment(guiRenderPassDef, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, gState.io.format, false);
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
		VkaImage swapchainImage = vkaGetSwapchainImage();
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
		if (1) // Gui button
		{
			appData.update(cmdBuf, appConfig);
		}
		// Render
		{
			if (1)
			{
				DrawCmd drawCmd{};
				setDefaults(drawCmd.pipelineDef, RasterizationPipelineDefaultValues());
				drawCmd.model           = modelCache.fetch(cmdBuf, "cube/cube.obj", sizeof(PosVertex), PosVertex::parse);
				drawCmd.instanceBuffers = {appData.gaussianFogCubeTransformBuf};
				drawCmd.instanceCount   = 1;
				addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				/*addDescriptor(drawCmd, &appData.envMapSamplerDef, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, lineColorImg, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, linePosImg, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);*/

				addInput(drawCmd.pipelineDef, PosVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
				addInput(drawCmd.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
				addShader(drawCmd.pipelineDef, shaderDir + "pins_render.vert", {});
				addShader(drawCmd.pipelineDef, shaderDir + "pins_render_gaussian_fog.frag",
				          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)}
					});
				//addDepthAttachment(drawCmd, depthImage, true, VK_TRUE, VK_COMPARE_OP_ALWAYS);
				addColorAttachment(drawCmd, offscreenImage, {1.0f,0.0f,0.0f,1.0f},
								   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
								   VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
				createFramebuffer(drawCmd, framebufferCache);
				vkaCmdDraw(cmdBuf, drawCmd);
			}

			// ...


			// Render gui
			vkaCmdEndRenderPass(cmdBuf);
			vkaCmdCopyImage(cmdBuf, offscreenImage, offscreenImage->getLayout(), swapchainImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			{
				vkaCmdStartRenderPass(cmdBuf, guiRenderPass, framebufferCache.fetch(guiRenderPass, {swapchainImage}), {VK_CLEAR_COLOR_BLACK});
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
#include <vka/vka.h>
#include "config.h"
#include "app_data.h"
#include "shaderStructs.h"

using namespace vka;
AppState gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
// clang-format off
std::vector<GVar *> gVars =
{
        &gvar_use_pins,
		&gvar_pin_selection_coef,
		&gvar_ray_lenght,
		&gvar_positional_jitter,
		&gvar_angular_jitter,
		&gvar_use_env_map,
		&gvar_reload
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
	ModelCache       modelCache       = ModelCache(&heap, modelPath);
	TextureCache     textureCache     = TextureCache(&heap, texturePath);
	AppData appData = AppData();


	appData.init(&heap);

	// Create Images
	VkaImage offscreenImage = vkaCreateSwapchainAttachment(
	    gState.io.format,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	    VK_IMAGE_LAYOUT_GENERAL);
	VkaImage depthImage = vkaCreateSwapchainAttachment(
	    VK_FORMAT_D32_SFLOAT,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	VkaImage lineColorImg = vkaCreateSwapchainAttachment(
	    gState.io.format,
	    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	    VK_IMAGE_LAYOUT_GENERAL);
	VkaImage linePosImg = vkaCreateSwapchainAttachment(
	    VK_FORMAT_R32G32B32A32_SFLOAT,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkaImage pinIdImage = vkaCreateSwapchainAttachment(
	    VK_FORMAT_R32_UINT,
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
			VkaImage envMap;
			if (1)
			{
				envMap = textureCache.fetch(cmdBuf,
					"autumn_field_2k.hdr",
					VK_FORMAT_R32G32B32A32_SFLOAT,
					VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			vkaCmdImageMemoryBarrier(cmdBuf, lineColorImg, VK_IMAGE_LAYOUT_GENERAL);
			// Render env map to offscreen image
			if (1)
			{
				ComputeCmd computeCmd;
				setDefaults(computeCmd, gState.io.extent, shaderPath + "render_env_map.comp");
				addDescriptor(computeCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				addDescriptor(computeCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER);
				addDescriptor(computeCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				addDescriptor(computeCmd, lineColorImg, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
				vkaCmdCompute(cmdBuf, computeCmd);
			}
			vkaCmdImageMemoryBarrier(cmdBuf, lineColorImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			// Wireframe Pass
			if (1)
			{
				DrawCmd drawCmdTemplate{};
				setDefaults(drawCmdTemplate.pipelineDef, RasterizationPipelineDefaultValues());
				addInput(drawCmdTemplate.pipelineDef, PosVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
				addInput(drawCmdTemplate.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
				addDepthAttachment(drawCmdTemplate, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
				addColorAttachment(drawCmdTemplate, lineColorImg, VK_CLEAR_COLOR_NONE,
				                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				                   VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
				addColorAttachment(drawCmdTemplate, linePosImg, {0.0f,0.0f,0.0f,0.0f},
				                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				                   VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
				createFramebuffer(drawCmdTemplate, framebufferCache);
				// Sphere
				{
					DrawCmd drawCmd = drawCmdTemplate;
					drawCmd.model           = modelCache.fetch(cmdBuf, "lowpoly_sphere/lowpoly_sphere.obj", sizeof(PosVertex), PosVertex::parse);
					drawCmd.instanceBuffers = {appData.sphereTransformBuf};
					drawCmd.instanceCount   = 1;

					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

					addShader(drawCmd.pipelineDef, shaderPath + "render_wireframe_sphere.vert");
					addShader(drawCmd.pipelineDef, shaderPath + "sphere_visualize.frag");

					drawCmd.pipelineDef.rasterizationState.cullMode  = VK_CULL_MODE_NONE;
					drawCmd.pipelineDef.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
					drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
					vkaCmdDraw(cmdBuf, drawCmd);
				}
				// Pins
				if (0)
				{
					DrawCmd drawCmd         = drawCmdTemplate;
					drawCmd.model           = {appData.pinVertexBuffer, appData.pinIndexBuffer, nullptr, 1};
					drawCmd.instanceBuffers = {appData.pinMatTransformBuf};
					drawCmd.instanceCount   = 1;

					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinUsedBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

					addShader(drawCmd.pipelineDef, shaderPath + "pins_visualize.vert");
					addShader(drawCmd.pipelineDef, shaderPath + "pins_visualize.frag",
						{{"PIN_COUNT", std::to_string(appConfig.pinCount())}});

					drawCmd.pipelineDef.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
					vkaCmdDraw(cmdBuf, drawCmd);
				}
				vkaCmdCopyImage(cmdBuf, lineColorImg, offscreenImage);
				vkaCmdFillBuffer(cmdBuf, appData.pinUsedBuffer, 0);
			}
			vkaCmdCopyImage(cmdBuf, lineColorImg, offscreenImage);
			// Fog Cube
			if (1)
			{
				DrawCmd drawCmd{};
				setDefaults(drawCmd.pipelineDef, RasterizationPipelineDefaultValues());
				drawCmd.model           = modelCache.fetch(cmdBuf, "cube/cube.obj", sizeof(PosVertex), PosVertex::parse);
				drawCmd.instanceBuffers = {appData.gaussianFogCubeTransformBuf};
				drawCmd.instanceCount   = 1;
				addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, lineColorImg, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, linePosImg, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

				addInput(drawCmd.pipelineDef, PosVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
				addInput(drawCmd.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);

				addShader(drawCmd.pipelineDef, shaderPath + "pins_render.vert", {});
				addShader(drawCmd.pipelineDef, shaderPath + "pins_render_gaussian_fog.frag",
				          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)}});

				addDepthAttachment(drawCmd, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
				addColorAttachment(drawCmd, offscreenImage, VK_CLEAR_COLOR_NONE,
				                   VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
								   VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
				createFramebuffer(drawCmd, framebufferCache);
				vkaCmdDraw(cmdBuf, drawCmd);
			}
			// Material Pass
			if (1)
			{
				DrawCmd drawCmdTemplate{};
				setDefaults(drawCmdTemplate.pipelineDef, RasterizationPipelineDefaultValues());
				drawCmdTemplate.model = modelCache.fetch(cmdBuf, "sphere/sphere.obj", sizeof(PosVertex), PosVertex::parse);
				addInput(drawCmdTemplate.pipelineDef, PosVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
				addInput(drawCmdTemplate.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
				addDepthAttachment(drawCmdTemplate, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
				addColorAttachment(drawCmdTemplate, offscreenImage, {0.9f, 0.9f, 0.9f, 0.0f},
				                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				                   VKA_BLEND_OP_WRITE, VKA_BLEND_OP_WRITE);
				drawCmdTemplate.renderArea = VkRect2D{{0, 0}, gState.io.extent};
				drawCmdTemplate.renderArea *= Rect2D<float>{0.8, 0.0, 0.2, 1.0};
				drawCmdTemplate.pipelineDef.rasterizationState.cullMode  = VK_CULL_MODE_BACK_BIT;
				drawCmdTemplate.pipelineDef.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
				addShader(drawCmdTemplate.pipelineDef, shaderPath + "secondary_pins_render.vert", {});

				createFramebuffer(drawCmdTemplate, framebufferCache);
				// Gaussian
				{
					DrawCmd drawCmd = drawCmdTemplate;
					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

					addShader(drawCmd.pipelineDef, shaderPath + "pins_render_gaussian.frag",
					          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)}});

					drawCmd.instanceBuffers = {appData.gaussianSphereTransformBuf};
					drawCmd.instanceCount   = 1;

					vkaCmdDraw(cmdBuf, drawCmd);
				}
				// Grid
				{
					DrawCmd drawCmd = drawCmdTemplate;
					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinTransmittanceBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

					addShader(drawCmd.pipelineDef, shaderPath + "pins_render_gaussian_nn_grid.frag",
					          {{"PIN_GRID_SIZE", std::to_string(appConfig.pinsGridSize)},
					           {"PIN_COUNT", std::to_string(appConfig.pinCount())},
					           {"PINS_PER_GRID_CELL", std::to_string(appConfig.pinsPerGridCell)},
					           {"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)}});

					drawCmd.instanceBuffers                          = {appData.gaussianNNGridSphereTransformBuf};
					drawCmd.instanceCount                            = 1;
					vkaCmdDraw(cmdBuf, drawCmd);
				}
				// NN
				if (1)
				{
					DrawCmd drawCmd = drawCmdTemplate;
					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinTransmittanceBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinDirectionsBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinUsedBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);


					addDescriptor(drawCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

					drawCmd.instanceCount = appConfig.pinCountSqrt;
					{
					    DrawCmd NN1 = drawCmd;
						addShader(NN1.pipelineDef, shaderPath + "pins_render_gaussian_nn.frag",
								  {{"PIN_COUNT", std::to_string(appConfig.pinCount())},
								   {"PIN_COUNT_SQRT", std::to_string(appConfig.pinCountSqrt)},
								   {"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
								   {"METRIC_ANGLE_DISTANCE", ""}
							});
						NN1.instanceBuffers = {appData.gaussianNNSphereTransformBuf};
						vkaCmdDraw(cmdBuf, NN1);
					}
					{
						DrawCmd NN2 = drawCmd;
						addShader(NN2.pipelineDef, shaderPath + "pins_render_gaussian_nn.frag",
						          {{"PIN_COUNT", std::to_string(appConfig.pinCount())},
						           {"PIN_COUNT_SQRT", std::to_string(appConfig.pinCountSqrt)},
						           {"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
						           {"METRIC_DISTANCE_DISTANCE", ""}});
						NN2.instanceBuffers = {appData.gaussianNN2SphereTransformBuf};
						vkaCmdDraw(cmdBuf, NN2);
					}
				}
			}

			vkaCmdCopyImage(cmdBuf, offscreenImage, offscreenImage->getLayout(), swapchainImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			// GUI
			if (1)
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
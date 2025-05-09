#include <vka/vka.h>
#include "config.h"
#include "app_data.h"
#include "shaderStructs.h"

#define MAP_APP_DATA(TYPE, NAME) TYPE *NAME = static_cast<TYPE *>(vkaMapStageing(appData.##NAME, sizeof(TYPE)));
#define PUSH_APP_DATA(CMD_BUF, NAME) \
	vkaUnmap(appData.##NAME);        \
	vkaCmdUpload(CMD_BUF, appData.##NAME);

#define APP_DATA_PUSH_DESC(CMD_BUF, TYPE, BUF_NAME, VAR_NAME)\
vkaWriteStaging(appData.##BUF_NAME, &appData.##VAR_NAME, sizeof(TYPE));\
vkaCmdUpload(CMD_BUF, appData.##BUF_NAME);

using namespace vka;
AppState gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
std::string       gShaderLog       = "";
    // clang-format off
std::vector<GVar *> gVars =
{
		&gvar_pin_selection_coef,
		&gvar_ray_lenght,
		&gvar_positional_jitter,
		&gvar_angular_jitter,
		&gvar_gaussian_weight,

		&gvar_gaussian_count,
		&gvar_gaussian_margin,
		&gvar_pins_per_grid_cell,
		&gvar_pins_grid_size,
		&gvar_pin_count_sqrt,
		&gvar_pin_transmittance_steps,
		&gvar_gaussian_std_deviation,
		&gvar_reload,

        &gvar_use_pins,
		&gvar_env_map,
		&gvar_show_cursor,

		&gvar_show_gaussian,
		&gvar_show_grid,
		&gvar_show_nn1,
		&gvar_show_nn2,
		&gvar_render_mode,
		&gvar_transmittance_mode,
		&gvar_volume_type,
		&gvar_accumulate,

		&gvar_perlin_scale0,
		&gvar_perlin_scale1,
		&gvar_perlin_frequency0,
		&gvar_perlin_frequency1,
		&gvar_perlin_falloff
};
// clang-format on


int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow    window   = GlfwWindow();
	gState.init(deviceCI, ioCI, &window);
	ImGuiWrapper     gui    = ImGuiWrapper();
	guiConfigDefault();
	// Init:
	VkaImage         swapchainImage   = vkaGetSwapchainImage();
	FramebufferCache framebufferCache = FramebufferCache();
	ResourcePool     heap             = ResourcePool();
	ModelCache       modelCache       = ModelCache(&heap, modelPath);
	TextureCache     textureCache     = TextureCache(&heap, texturePath);
	AppData appData = AppData();
	FastDrawState fastDrawState = FastDrawState(&framebufferCache, gState.cache);

	appData.init(&heap, &textureCache, &fastDrawState);

	// Create Images
	VkaImage offscreenImage = vkaCreateSwapchainAttachment(
	    VK_FORMAT_R32G32B32A32_SFLOAT,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkaImage depthImage = vkaCreateSwapchainAttachment(
	    VK_FORMAT_D32_SFLOAT,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	VkaImage lineColorImg = vkaCreateSwapchainAttachment(
	    VK_FORMAT_R32G32B32A32_SFLOAT,
	    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	    VK_IMAGE_LAYOUT_GENERAL);
	VkaImage linePosImg = vkaCreateSwapchainAttachment(
	    VK_FORMAT_R32G32B32A32_SFLOAT,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkaImage pinIdImage = vkaCreateSwapchainAttachment(
	    VK_FORMAT_R32_UINT,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	// New
	VkaImage transmittanceImage = vkaCreateSwapchainAttachment(
		VK_FORMAT_R32_SFLOAT,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	

	// Create Defaul Render Passes
	VkRenderPass swapchainLoadRP, swapchainClearRP, hdrAttachmentClearRP;
	{
		RenderPassDefinition swapchainLoadRPDef = RenderPassDefinition();
		setDefaults(swapchainLoadRPDef);
		addColorAttachment(swapchainLoadRPDef, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, gState.io.format, false);
		swapchainLoadRP = gState.cache->fetch(swapchainLoadRPDef);
	}
	{
		RenderPassDefinition swapchainClearRPDef = RenderPassDefinition();
		setDefaults(swapchainClearRPDef);
		addColorAttachment(swapchainClearRPDef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, gState.io.format, true);
		swapchainClearRP = gState.cache->fetch(swapchainClearRPDef);
	}
	{
		RenderPassDefinition hdrAttachmentClearRPDef = RenderPassDefinition();
		setDefaults(hdrAttachmentClearRPDef);
		addColorAttachment(hdrAttachmentClearRPDef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_FORMAT_R32G32B32A32_SFLOAT, true);
		hdrAttachmentClearRP = gState.cache->fetch(hdrAttachmentClearRPDef);
	}
	gui.vkaImGuiInit(swapchainLoadRP, 0);
	
	
	
	AppConfig appConfig = AppConfig();
	VkaCommandBuffer cmdBuf = vkaCreateCommandBuffer(gState.frame->stack);
	gState.io.swapchainAttachmentPool->refreshImages(cmdBuf);
	appData.update(cmdBuf, appConfig);
	gui.vkaImGuiUpload(cmdBuf);

	
	vkaExecuteImmediat(cmdBuf);
	gui.vkaImGuiFreeStaging();

	// Main loop:
	while (!gState.io.shouldTerminate())
	{
		gui.vkaImGuiNewFrame();
		VkaImage swapchainImage = vkaGetSwapchainImage();
		// Reload shaders
		if (gState.io.keyEvent[GLFW_KEY_R] && gState.io.keyPressed[GLFW_KEY_R])
		{
			vkDeviceWaitIdle(gState.device.logical);
			gState.cache->clearShaders();
			gShaderLog = "";
		}
		// Resize window
		if (gState.io.swapchainRecreated())
		{
			framebufferCache.clear();
		}

		VkaCommandBuffer cmdBuf = vkaCreateCommandBuffer(gState.frame->stack);
		{
			appConfig.update();
			appData.update(cmdBuf, appConfig);
		}

		if (gvar_render_mode.val.v_int == 0)
		{
			vkaClearState(cmdBuf);
			vkaCmdStartRenderPass(cmdBuf, hdrAttachmentClearRP, framebufferCache.fetch(hdrAttachmentClearRP, {offscreenImage}), {{1.f, 1.f, 1.f, 1.f}}, appConfig.mainViewport);
			vkaCmdEndRenderPass(cmdBuf);

			// Render env map to offscreen image
			vkaCmdImageMemoryBarrier(cmdBuf, lineColorImg, VK_IMAGE_LAYOUT_GENERAL);
			if (1)
			{
				ComputeCmd computeCmd;
				setDefaults(computeCmd, gState.io.extent, shaderPath + "render_env_map.comp");
				addDescriptor(computeCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				addDescriptor(computeCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER);
				addDescriptor(computeCmd, appData.envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				addDescriptor(computeCmd, lineColorImg, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
				vkaCmdCompute(cmdBuf, computeCmd);
			}
			vkaCmdImageMemoryBarrier(cmdBuf, lineColorImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			// Borders
			


			DrawCmd drawCmdMainWindow{};
			setDefaults(drawCmdMainWindow.pipelineDef, RasterizationPipelineDefaultValues());
			drawCmdMainWindow.renderArea = appConfig.mainViewport;
			// Wireframe Pass
			if (1)
			{
				DrawCmd drawCmdWireframe = drawCmdMainWindow;
				addInput(drawCmdWireframe.pipelineDef, PosVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
				addInput(drawCmdWireframe.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
				addDepthAttachment(drawCmdWireframe, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
				addColorAttachment(drawCmdWireframe, lineColorImg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				addColorAttachment(drawCmdWireframe, linePosImg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {0.0f, 0.0f, 0.0f, 0.0f});
				createFramebuffer(drawCmdWireframe, framebufferCache);
				// Sphere
				if (gvar_show_cursor.val.v_int)
				{
					DrawCmd drawCmd         = drawCmdWireframe;
					drawCmd.model           = modelCache.fetch(cmdBuf, "lowpoly_sphere/lowpoly_sphere.obj", PosVertex::parseObj);
					drawCmd.instanceBuffers = {appData.sphereTransformBuf};
					drawCmd.instanceCount   = gvar_show_cursor.val.v_int;

					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

					addShader(drawCmd.pipelineDef, shaderPath + "render_wireframe_sphere.vert");
					addShader(drawCmd.pipelineDef, shaderPath + "sphere_visualize.frag");

					drawCmd.pipelineDef.rasterizationState.cullMode  = VK_CULL_MODE_NONE;
					drawCmd.pipelineDef.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
					//drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
					vkaCmdDraw(cmdBuf, drawCmd);
				}

				// Pins
				if (1)
				{
					DrawCmd drawCmd         = drawCmdWireframe;
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
			// Fog Cube
			if (1)
			{
				DrawCmd drawCmd = drawCmdMainWindow;
				drawCmd.model           = modelCache.fetch(cmdBuf, "cube/cube.obj", PosVertex::parseObj);
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
				addColorAttachment(drawCmd, offscreenImage);
				createFramebuffer(drawCmd, framebufferCache);
				vkaCmdDraw(cmdBuf, drawCmd);
			}
			// Material Pass
			if (1)
			{
				vkaClearState(cmdBuf);
				vkaCmdStartRenderPass(cmdBuf, hdrAttachmentClearRP, framebufferCache.fetch(hdrAttachmentClearRP, {offscreenImage}), {{0.f, 0.f, 0.f, 0.f}}, vkaGetScissorRect(0.2, 0.8));
				vkaCmdEndRenderPass(cmdBuf);


				DrawCmd drawCmdTemplate{};
				setDefaults(drawCmdTemplate.pipelineDef, RasterizationPipelineDefaultValues());
				drawCmdTemplate.model = modelCache.fetch(cmdBuf, "sphere/sphere.obj", PosVertex::parseObj);
				addInput(drawCmdTemplate.pipelineDef, PosVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
				addInput(drawCmdTemplate.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
				addDepthAttachment(drawCmdTemplate, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
				addColorAttachment(drawCmdTemplate, offscreenImage, {0.5f, 0.5f, 0.5f, 0.0f});
				addColorAttachment(drawCmdTemplate, pinIdImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {0, 0, 0, 0});
				drawCmdTemplate.renderArea                               = appConfig.materialViewPort;
				drawCmdTemplate.pipelineDef.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
				drawCmdTemplate.pipelineDef.rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
				addShader(drawCmdTemplate.pipelineDef, shaderPath + "secondary_pins_render.vert", {});

				createFramebuffer(drawCmdTemplate, framebufferCache);
				// Gaussian
				if (gvar_show_gaussian.val.v_bool)
				{
					DrawCmd drawCmd = drawCmdTemplate;
					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

					addShader(drawCmd.pipelineDef, shaderPath + "pins_render_gaussian.frag",
						{ {"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)} });

					drawCmd.instanceBuffers = { appData.gaussianSphereTransformBuf };
					drawCmd.instanceCount = 1;

					vkaCmdDraw(cmdBuf, drawCmd);
				}
				// Grid
				if (gvar_show_grid.val.v_bool)
				{
					DrawCmd drawCmd = drawCmdTemplate;
					addDescriptor(drawCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinTransmittanceBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

					addShader(drawCmd.pipelineDef, shaderPath + "pins_render_gaussian_nn_grid.frag",
						{ {"PIN_GRID_SIZE", std::to_string(appConfig.pinsGridSize)},
						 {"PIN_COUNT", std::to_string(appConfig.pinCount())},
						 {"PINS_PER_GRID_CELL", std::to_string(appConfig.pinsPerGridCell)},
						 {"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)} });

					drawCmd.instanceBuffers = { appData.gaussianNNGridSphereTransformBuf };
					drawCmd.instanceCount = 1;
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
					addDescriptor(drawCmd, appData.envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

					drawCmd.instanceCount = appConfig.pinCountSqrt;
					if (gvar_show_nn1.val.v_bool)
					{
						DrawCmd NN1 = drawCmd;
						addShader(NN1.pipelineDef, shaderPath + "pins_render_gaussian_nn.frag",
							{ {"PIN_COUNT", std::to_string(appConfig.pinCount())},
							 {"PIN_COUNT_SQRT", std::to_string(appConfig.pinCountSqrt)},
							 {"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
							 {"METRIC_ANGLE_DISTANCE", ""}
							});
						NN1.instanceBuffers = { appData.gaussianNNSphereTransformBuf };
						vkaCmdDraw(cmdBuf, NN1);
					}
					if (gvar_show_nn2.val.v_bool)
					{
						DrawCmd NN2 = drawCmd;
						addShader(NN2.pipelineDef, shaderPath + "pins_render_gaussian_nn.frag",
							{ {"PIN_COUNT", std::to_string(appConfig.pinCount())},
							 {"PIN_COUNT_SQRT", std::to_string(appConfig.pinCountSqrt)},
							 {"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
							 {"METRIC_DISTANCE_DISTANCE", ""} });
						NN2.instanceBuffers = { appData.gaussianNN2SphereTransformBuf };
						vkaCmdDraw(cmdBuf, NN2);
					}
				}
			}

			// Update used pins
			if (1)
			{
				ComputeCmd computeCmd;
				setDefaults(computeCmd, gState.io.extent, shaderPath + "write_used_pins.comp", {{"PIN_COUNT", std::to_string(appConfig.pinCount())}});
				addDescriptor(computeCmd, appData.viewBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				addDescriptor(computeCmd, appData.pinUsedBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				addDescriptor(computeCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER);
				addDescriptor(computeCmd, pinIdImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				vkaCmdCompute(cmdBuf, computeCmd);
			}
		}
		if (gvar_render_mode.val.v_int == 1)
		{
			vkaClearState(cmdBuf);
			vkaCmdStartRenderPass(cmdBuf, hdrAttachmentClearRP, framebufferCache.fetch(hdrAttachmentClearRP, {offscreenImage}), {{1.f, 1.f, 1.f, 1.f}}, appConfig.mainViewport);
			vkaCmdEndRenderPass(cmdBuf);

			DrawCmd drawCmdMainWindow{};
			setDefaults(drawCmdMainWindow.pipelineDef, RasterizationPipelineDefaultValues());
			addDepthAttachment(drawCmdMainWindow, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
			addColorAttachment(drawCmdMainWindow, offscreenImage);
			createFramebuffer(drawCmdMainWindow, framebufferCache);
			addInput(drawCmdMainWindow.pipelineDef, PosNormalVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
			addInput(drawCmdMainWindow.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
			drawCmdMainWindow.renderArea = appConfig.mainViewport;


			APP_DATA_PUSH_DESC(cmdBuf, CamConst, camConstBuf, mainCamConst);
			APP_DATA_PUSH_DESC(cmdBuf, ViewConst, viewConstBuf, mainViewConst);
			APP_DATA_PUSH_DESC(cmdBuf, GuiVar, guiVarBuf, guiVar);
			APP_DATA_PUSH_DESC(cmdBuf, Transform, cubeTransformBuf, cubeTransform);

			vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			DrawCmd drawCmd         = drawCmdMainWindow;
			drawCmd.model           = modelCache.fetch(cmdBuf, "sphere/sphere.obj", PosNormalVertex::parseObj);
			drawCmd.instanceBuffers = {appData.sphereTransformBuf};
			addDescriptor(drawCmd, appData.camConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.viewConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.guiVarBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.cubeTransformBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
			
			drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
			drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
			drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			
			// Exact
			if (gvar_transmittance_mode.val.v_int == 0)
			{
				drawCmd.instanceCount   = 1;
				addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/gaussian/exact.frag",
				          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)}});
				vkaCmdDraw(cmdBuf, drawCmd);
			}
			// Grid
			if (gvar_transmittance_mode.val.v_int == 1)
			{
				drawCmd.instanceCount = 1;        // appConfig.pinCountSqrt;
				addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, appData.pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/gaussian/grid.frag",
				          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
				           {"PIN_GRID_SIZE", std::to_string(appConfig.pinsGridSize)},
				           {"PINS_PER_GRID_CELL", std::to_string(appConfig.pinsPerGridCell)}});
				vkaCmdDraw(cmdBuf, drawCmd);
			}
			// NN1
			if (gvar_transmittance_mode.val.v_int == 2)
			{
				drawCmd.instanceCount   = appConfig.pinCountSqrt;
				addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, appData.pinBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/gaussian/nearest_neighbor.frag",
				          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
				           {"PIN_COUNT", std::to_string(appConfig.pinCount())},
				           {"PIN_COUNT_SQRT", std::to_string(appConfig.pinCountSqrt)},
				           {"METRIC_ANGLE_DISTANCE", ""}});
				vkaCmdDraw(cmdBuf, drawCmd);
			}
			// NN2
			if (gvar_transmittance_mode.val.v_int == 3)
			{
				drawCmd.instanceCount = appConfig.pinCountSqrt;
				addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addDescriptor(drawCmd, appData.pinBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
				addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/gaussian/nearest_neighbor.frag",
				          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
				           {"PIN_COUNT", std::to_string(appConfig.pinCount())},
				           {"PIN_COUNT_SQRT", std::to_string(appConfig.pinCountSqrt)},
				           {"METRIC_DISTANCE_DISTANCE", ""}});
				vkaCmdDraw(cmdBuf, drawCmd);
			}
			if (1)
			{
				DrawCmd drawCmd         = drawCmdMainWindow;
				drawCmd.model           = modelCache.fetch(cmdBuf, "cube/cube.obj", PosNormalVertex::parseObj);
				drawCmd.instanceBuffers = {appData.gaussianFogCubeTransformBuf};
				drawCmd.instanceCount   = 1;
				addDescriptor(drawCmd, appData.camConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
				drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_NONE;
				drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
				drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
				addShader(drawCmd.pipelineDef, newShaderPath + "shading.vert", {});
				addShader(drawCmd.pipelineDef, newShaderPath + "shading.frag", {});
				vkaCmdDraw(cmdBuf, drawCmd);
			}





			// Pre fog passes:
			// Pos,Color <--- Pins, Sphere, Shaded Obj
			{

			}
			
			// Primary Ray Fog pass:
			{

			}
		}
		if (gvar_render_mode.val.v_int == 2)
		{

			if (gvar_env_map.val.v_uint != 0)
			{

				fastDrawState.drawEnvMap(cmdBuf, offscreenImage, appData.envMap, appData.defaultSampler, appData.camera.getViewMatrix(), appConfig.mainProjectionMat, appConfig.mainViewport);
			}
			else
			{
				vkaClearState(cmdBuf);
				vkaCmdStartRenderPass(cmdBuf, hdrAttachmentClearRP, framebufferCache.fetch(hdrAttachmentClearRP, {offscreenImage}), {{1.f, 1.f, 1.f, 1.f}}, appConfig.mainViewport);
				vkaCmdEndRenderPass(cmdBuf);
			}


			DrawCmd drawCmdMainWindow{};
			setDefaults(drawCmdMainWindow.pipelineDef, RasterizationPipelineDefaultValues());
			addDepthAttachment(drawCmdMainWindow, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
			addColorAttachment(drawCmdMainWindow, offscreenImage);
			createFramebuffer(drawCmdMainWindow, framebufferCache);
			addInput(drawCmdMainWindow.pipelineDef, PosNormalVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
			addInput(drawCmdMainWindow.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
			drawCmdMainWindow.renderArea = appConfig.mainViewport;

			APP_DATA_PUSH_DESC(cmdBuf, CamConst, camConstBuf, mainCamConst);
			APP_DATA_PUSH_DESC(cmdBuf, ViewConst, viewConstBuf, mainViewConst);
			APP_DATA_PUSH_DESC(cmdBuf, GuiVar, guiVarBuf, guiVar);
			APP_DATA_PUSH_DESC(cmdBuf, Transform, cubeTransformBuf, cubeTransform);

			vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			DrawCmd drawCmd         = drawCmdMainWindow;
			drawCmd.model           = modelCache.fetch(cmdBuf, "cube/cube.obj", PosNormalVertex::parseObj);
			drawCmd.instanceBuffers = {appData.gaussianFogCubeTransformBuf};
			addDescriptor(drawCmd, appData.camConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.viewConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.guiVarBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.cubeTransformBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
			// Envmap
			addDescriptor(drawCmd, &appData.defaultSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

		    addDescriptor(drawCmd, appData.pdfHorizontal, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
			addDescriptor(drawCmd, appData.pdfVertical, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

			drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
			drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
			drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			drawCmd.instanceCount = 1;
			// Exact
			if (gvar_transmittance_mode.val.v_int == 0)
			{
				if (gvar_volume_type.val.v_int == 0)
				{
					addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/gaussian/fog_cube.frag",
					          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)}});
				}
				else
				{
					addDescriptor(drawCmd, appData.volumeImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, &appData.volumeDataSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/ray_marched/fog_cube.frag");
				}
				vkaCmdDraw(cmdBuf, drawCmd);
			}
			// Grid
			if (gvar_transmittance_mode.val.v_int == 1)
			{
				if (gvar_volume_type.val.v_int == 0)
				{
					addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/gaussian/fog_cube_grid.frag",
							  {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)},
							   {"PIN_GRID_SIZE", std::to_string(appConfig.pinsGridSize)},
							   {"PINS_PER_GRID_CELL", std::to_string(appConfig.pinsPerGridCell)}});
				}
				else
				{
					addDescriptor(drawCmd, appData.volumeImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, &appData.volumeDataSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinGridBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addDescriptor(drawCmd, appData.pinTransmittanceBufV2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
					addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/ray_marched/fog_cube_grid.frag",
					          {{"PIN_GRID_SIZE", std::to_string(appConfig.pinsGridSize)},
					           {"PINS_PER_GRID_CELL", std::to_string(appConfig.pinsPerGridCell)},
					           {"PIN_COUNT", std::to_string(appConfig.pinCount())},
					           {"PIN_TRANSMITTANCE_STEPS", std::to_string(appConfig.pinTransmittanceSteps)},
						});
				}
				vkaCmdDraw(cmdBuf, drawCmd);
			}
		}

		// Tools
		{
			vkaCmdTransitionLayout(cmdBuf, offscreenImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			vkaCmdTransitionLayout(cmdBuf, swapchainImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			VkRect2D_OP scissor = {0, 0, offscreenImage->getExtent2D().width, offscreenImage->getExtent2D().height};
			fastDrawState.renderSprite(cmdBuf, offscreenImage, appData.defaultSampler, swapchainImage, scissor);
			// Histogram
			{
				if (appData.regionSelected && !appData.histogramLoaded)
				{
					// Load histogram -> appData.histogramImage
					appData.histogramLoaded = true;
				}
				if (appData.startRegionSelect)
				{
					glm::uvec2  regionEnd  = gState.io.mouse.pos;
					glm::uvec2  upperLeft  = glm::min(appData.regionStart, regionEnd);
					glm::uvec2  lowerRight = glm::max(appData.regionStart, regionEnd);
					glm::uvec2  size       = lowerRight - upperLeft;
					VkRect2D_OP scissor    = {upperLeft.x, upperLeft.y, size.x, size.y};
					if (scissor.isValid(swapchainImage->getExtent2D()))
					{
						fastDrawState.drawRect(cmdBuf, swapchainImage, {0.2f, 0.2f, 0.8f, 0.2f}, scissor);
					}
				}
				else if (appData.histogramLoaded)
				{
					glm::uvec2  upperLeft  = glm::min(appData.regionStart, appData.regionEnd);
					glm::uvec2  lowerRight = glm::max(appData.regionStart, appData.regionEnd);
					glm::uvec2  size       = lowerRight - upperLeft;
					VkRect2D_OP scissor    = {upperLeft.x, upperLeft.y, size.x, size.y};
					if (scissor.isValid(swapchainImage->getExtent2D()))
					{
						fastDrawState.drawRect(cmdBuf, swapchainImage, {0.2f, 0.2f, 0.8f, 0.2f}, scissor);
						fastDrawState.computeHistogram(cmdBuf, offscreenImage, &appData.defaultSampler, appData.histogramBuffer, appData.histogramAverageBuffer, scissor);
						vkaCmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
						fastDrawState.renderHistogram(cmdBuf, appData.histogramBuffer, appData.histogramAverageBuffer, swapchainImage, scissor);
					}
				}
			}

			// Accumulation

			if (gvar_accumulate.val.v_bool)
			{
				bool resetAccumulationLastFrame = appData.resetAccumulation;
				appData.resetAccumulation =
				    // clang-format off
					gState.io.mouse.leftPressed
					|| gState.io.mouse.rightPressed
					|| gState.io.mouse.scrollChange != 0.0
					|| gState.io.keyPressed[GLFW_KEY_W]
					|| gState.io.keyPressed[GLFW_KEY_A]
					|| gState.io.keyPressed[GLFW_KEY_S]
					|| gState.io.keyPressed[GLFW_KEY_D]
					|| gState.io.keyPressed[GLFW_KEY_SPACE]
					|| gState.io.keyPressed[GLFW_KEY_LEFT_SHIFT];
				// clang-format on
				if (!appData.resetAccumulation)
				{
					appData.accumulationImage->changeExtent(getExtent3D(scissor.extent));
					appData.accumulationImage->recreate();

					if (resetAccumulationLastFrame)
					{
						appData.accumulationCount = 1;
					}

					vkaCmdTransitionLayout(cmdBuf, appData.accumulationImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
					fastDrawState.accumulate(cmdBuf, offscreenImage, appData.defaultSampler, appData.accumulationImage, scissor, appData.accumulationCount++);
					vkaCmdTransitionLayout(cmdBuf, appData.accumulationImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					fastDrawState.renderSprite(cmdBuf, appData.accumulationImage, appData.defaultSampler, swapchainImage, scissor);
				}
			}
			else
			{
				if (appData.regionSelectedAccum && !appData.accumulationLoaded)
				{
					// Load histogram -> appData.histogramImage
					appData.accumulationLoaded = true;
				}
				if (appData.startRegionSelectAccum)
				{
					glm::uvec2  regionEnd  = gState.io.mouse.pos;
					glm::uvec2  upperLeft  = glm::min(appData.regionStartAccum, regionEnd);
					glm::uvec2  lowerRight = glm::max(appData.regionStartAccum, regionEnd);
					glm::uvec2  size       = lowerRight - upperLeft;
					VkRect2D_OP scissor    = {upperLeft.x, upperLeft.y, size.x, size.y};
					if (scissor.isValid(swapchainImage->getExtent2D()))
					{
						fastDrawState.drawRect(cmdBuf, swapchainImage, {0.8f, 0.2f, 0.2f, 0.2f}, scissor);
					}
				}
				else if (appData.accumulationLoaded)
				{
					glm::uvec2  upperLeft  = glm::min(appData.regionStartAccum, appData.regionEndAccum);
					glm::uvec2  lowerRight = glm::max(appData.regionStartAccum, appData.regionEndAccum);
					glm::uvec2  size       = lowerRight - upperLeft;
					VkRect2D_OP scissor    = {upperLeft.x, upperLeft.y, size.x, size.y};

					if (scissor.isValid(swapchainImage->getExtent2D()))
					{
						appData.accumulationImage->changeExtent(getExtent3D(scissor.extent));
						appData.accumulationImage->recreate();

						vkaCmdTransitionLayout(cmdBuf, appData.accumulationImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
						fastDrawState.accumulate(cmdBuf, offscreenImage, appData.defaultSampler, appData.accumulationImage, scissor, appData.accumulationCount++);
						vkaCmdTransitionLayout(cmdBuf, appData.accumulationImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						fastDrawState.renderSprite(cmdBuf, appData.accumulationImage, appData.defaultSampler, swapchainImage, scissor);
					}
				}
			}
			// Visualize Importance Sampling
			{
				if (gState.io.keyPressed[GLFW_KEY_T])
				{
					fastDrawState.renderDistribution(cmdBuf, appData.pdfHorizontal, 64, swapchainImage, appConfig.mainViewport);
				}
				if (gState.io.keyPressed[GLFW_KEY_G])
				{
					fastDrawState.renderDistribution(cmdBuf, appData.pdfVertical, 64, swapchainImage, appConfig.mainViewport);
				}
			}
		}

		vkaCmdTransitionLayout(cmdBuf, offscreenImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// GUI
		{
			vkaClearState(cmdBuf);
			vkaCmdStartRenderPass(cmdBuf, swapchainClearRP, framebufferCache.fetch(swapchainClearRP, {swapchainImage}), {{0.2f, 0.2f, 0.2f, 0.0f}}, vkaGetScissorRect(0.0, 0.0, 0.2, 1.0));
			vkaCmdEndRenderPass(cmdBuf);
			vkaCmdStartRenderPass(cmdBuf, swapchainLoadRP, framebufferCache.fetch(swapchainLoadRP, {swapchainImage}));
			gvar_gui::buildGui(appConfig.guiViewport);
			if (gShaderLog != "")
			{
				shader_console_gui::buildGui(appConfig.mainViewport);
			}
			gui.vkaImGuiRender(cmdBuf);
			vkaCmdEndRenderPass(cmdBuf);
		}
		
		vkaSwapBuffers({cmdBuf});
	}
	// Cleanup
	vkDeviceWaitIdle(gState.device.logical);
	framebufferCache.clear();
	heap.clear();
	gui.vkaImGuiDestroy();
	gState.destroy();
}
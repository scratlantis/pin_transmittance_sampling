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
		&gvar_reload,

        &gvar_use_pins,
		&gvar_use_env_map,
		&gvar_show_cursor,

		&gvar_show_gaussian,
		&gvar_show_grid,
		&gvar_show_nn1,
		&gvar_show_nn2,
		&gvar_render_mode,
		&gvar_transmittance_mode
};
// clang-format on
int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
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


	appData.init(&heap);

	// Create Images
	VkaImage offscreenImage = vkaCreateSwapchainAttachment(
	    gState.io.format,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
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

	// New
	VkaImage transmittanceImage = vkaCreateSwapchainAttachment(
		VK_FORMAT_R32_SFLOAT,
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);


	VkRenderPass swapchainLoadRP, swapchainClearRP, attachmentClearRP;

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
		RenderPassDefinition attachmentClearRPDef = RenderPassDefinition();
		setDefaults(attachmentClearRPDef);
		addColorAttachment(attachmentClearRPDef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, gState.io.format, true);
		attachmentClearRP = gState.cache->fetch(attachmentClearRPDef);
	}
	/*RenderPassDefinition attachmentClearRenderPassDef = RenderPassDefinition();
	setDefaults(attachmentClearRenderPassDef);
	addColorAttachment(attachmentClearRenderPassDef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, gState.io.format, true);*/

	/*VkRenderPass         swapchainRenderPass    = gState.cache->fetch(swapchainLoadRP);
	VkRenderPass attachmentClearRenderPass = gState.cache->fetch(attachmentClearRenderPassDef);*/
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
		if (1) // Gui button
		{
			appConfig.update();
			appData.update(cmdBuf, appConfig);
		}
		// Render probe
		
		if (gvar_render_mode.val.v_int == 0)
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
					drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
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
				vkaCmdStartRenderPass(cmdBuf, attachmentClearRP, framebufferCache.fetch(attachmentClearRP, {offscreenImage}), {{0.f, 0.f, 0.f, 0.f}}, vkaGetScissorRect(0.2, 0.8));
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
					addDescriptor(drawCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

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
					addDescriptor(drawCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

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
					addDescriptor(drawCmd, envMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT);

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
			vkaCmdStartRenderPass(cmdBuf, attachmentClearRP, framebufferCache.fetch(attachmentClearRP, {offscreenImage}), {{1.f, 1.f, 1.f, 1.f}}, appConfig.mainViewport);
			vkaCmdEndRenderPass(cmdBuf);

			DrawCmd drawCmdMainWindow{};
			setDefaults(drawCmdMainWindow.pipelineDef, RasterizationPipelineDefaultValues());
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
			addInput(drawCmd.pipelineDef, PosNormalVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
			addInput(drawCmd.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
			addDepthAttachment(drawCmd, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
			addColorAttachment(drawCmd, offscreenImage);
			createFramebuffer(drawCmd, framebufferCache);
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
				drawCmd.instanceCount = appConfig.pinCountSqrt;
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
			{
				DrawCmd drawCmd         = drawCmdMainWindow;
				drawCmd.model           = modelCache.fetch(cmdBuf, "cube/cube.obj", PosNormalVertex::parseObj);
				drawCmd.instanceBuffers = {appData.gaussianFogCubeTransformBuf};
				drawCmd.instanceCount   = 1;
				addDescriptor(drawCmd, appData.camConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
				addInput(drawCmd.pipelineDef, PosNormalVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
				addInput(drawCmd.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
				addDepthAttachment(drawCmd, depthImage, true, VK_TRUE, VK_COMPARE_OP_NEVER);
				addColorAttachment(drawCmd, offscreenImage);
				createFramebuffer(drawCmd, framebufferCache);
				drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
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
		vkaCmdCopyImage(cmdBuf, offscreenImage, offscreenImage->getLayout(), swapchainImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		
		if (appData.regionSelected && !appData.histogramLoaded)
		{
			// Load histogram -> appData.histogramImage
			appData.histogramLoaded = true;
		}
		if (appData.startRegionSelect)
		{
			glm::uvec2                regionEnd = gState.io.mouse.pos;
			glm::uvec2 upperLeft = glm::min(appData.regionStart,  regionEnd);
			glm::uvec2 lowerRight = glm::max(appData.regionStart,  regionEnd);
			glm::uvec2 size = lowerRight - upperLeft;
			VkRect2D_OP scissor    = {upperLeft.x, upperLeft.y, size.x, size.y};
			
			/*vkaCmdStartRenderPass(cmdBuf, swapchainClearRP, framebufferCache.fetch(swapchainClearRP, {swapchainImage}), {{0.2f, 0.2f, 0.2f, 0.0f}}, scissor);
			vkaCmdEndRenderPass(cmdBuf);*/
			if (size.x != 0 && size.y != 0 && upperLeft.x >= 0 && upperLeft.y >= 0 && lowerRight.x < swapchainImage->getExtent().width && lowerRight.y < swapchainImage->getExtent().height)
			{
				vkaCmdStartRenderPass(cmdBuf, swapchainClearRP, framebufferCache.fetch(swapchainClearRP, {swapchainImage}), {{0.2f, 0.2f, 0.2f, 0.0f}}, scissor);
				//vkaCmdFill(cmdBuf, glm::vec4(0.0));
				vkaCmdEndRenderPass(cmdBuf);
			}
		}
		else if (appData.histogramLoaded)
		{
			glm::uvec2  upperLeft  = glm::min(appData.regionStart, appData.regionEnd);
			glm::uvec2  lowerRight = glm::max(appData.regionStart, appData.regionEnd);
			glm::uvec2  size       = lowerRight - upperLeft;
			VkRect2D_OP scissor    = {upperLeft.x, upperLeft.y, size.x, size.y};

			if (size.x != 0 && size.y != 0 && upperLeft.x >= 0 && upperLeft.y >= 0 && lowerRight.x < swapchainImage->getExtent().width && lowerRight.y < swapchainImage->getExtent().height)
			{
				vkaCmdStartRenderPass(cmdBuf, swapchainClearRP, framebufferCache.fetch(swapchainClearRP, {swapchainImage}), {{0.2f, 0.2f, 0.2f, 0.0f}}, scissor);
				//vkaCmdFill(cmdBuf, glm::vec4(0.0));
				vkaCmdEndRenderPass(cmdBuf);
			}
		}

		// GUI
		if (1)
		{
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
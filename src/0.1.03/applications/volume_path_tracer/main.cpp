#include "config.h"
#include "utility.h"
#include "shader_interface.h"
#include "medium/Medium.h"
#include "medium/medium_strategies.h"
#include "visualization/pin_visualization.h"
#include "path_tracing/ComparativePathTracer.h"
#include "path_tracing/pt_strategies.h"
#include "EventManager.h"
// State and output dir must be defined in the main file
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

// Gui variable
GVar gvar_model = {"Model", 0, GVAR_ENUM, NO_GUI, std::vector<std::string>{"Cornell Box", "Sponza"}};
GVar gvar_image_resolution{"Image Resolution", 64, GVAR_UINT_RANGE, NOISE_SETTINGS, {16, 256}};
GVar gvar_mse{"MSE : %.8f E-3", 0.0f, GVAR_DISPLAY_VALUE, METRICS };
GVar gvar_env_map                   = {"Envmap", 1, GVAR_ENUM, NO_GUI, std::vector<std::string>{"None"}};
GVar gvar_fixed_seed = {"Fixed Seed", false, GVAR_BOOL, PATH_TRACING};
GVar gvar_seed                      = {"Seed", 0, GVAR_UINT_RANGE, PATH_TRACING, {0, 1000}};
GVar gvar_medium_xray_line_segments = {"Medium Xray Line Segments", true, GVAR_BOOL, VISUALIZATION_SETTINGS};
GVar gvar_pin_sample_location       = {"Pin sample location", 0, GVAR_UNORM, PIN_SETTINGS};
GVar gvar_continuous_path_sampling  = {"Contious Path Sampling", false, GVAR_BOOL, VISUALIZATION_SETTINGS};

GVar gvar_path_sampling_event       = {"Path Sampling Event", false, GVAR_EVENT, NO_GUI};
GVar gvar_screen_cursor_pos = {"Screen Cursor Pos", {0.f,0.f,0.f}, GVAR_VEC3, NO_GUI};
GVar gvar_screen_cursor_seed        = {"Screen Cursor Seed", 0, GVAR_UINT, NO_GUI};
GVar gvar_screen_cursor_enable = {"Screen Cursor Enable", false, GVAR_BOOL, NO_GUI};

GVar gvar_select_config    = {"Select Config", 0, GVAR_ENUM, NO_GUI, std::vector<std::string>{"Default"}, GUI_FLAGS_NO_LOAD};
GVar gvar_save_config      = {"Save Config", false, GVAR_EVENT, NO_GUI, GUI_FLAGS_NO_LOAD};
GVar gvar_reload_config    = {"Load Config", false, GVAR_EVENT, NO_GUI, GUI_FLAGS_NO_LOAD};
GVar gvar_save_config_name = {"Save Config Name", std::string("quicksave"), GVAR_TEXT_INPUT, NO_GUI, GUI_FLAGS_NO_LOAD};

std::vector<GVar *> gVars =
{
        // clang-format off
        &gvar_model,
		&gvar_perlin_frequency0,
		&gvar_perlin_frequency1,
		&gvar_perlin_scale0,
		&gvar_perlin_scale1,
		&gvar_perlin_falloff,
		&gvar_medium_albedo,
		&gvar_image_resolution,
		&gvar_pin_count,
		&gvar_pin_transmittance_value_count,
		&gvar_pin_count_per_grid_cell,
		&gvar_pin_grid_size,
		&gvar_cursor_pos,
		&gvar_cursor_dir_phi,
		&gvar_cursor_dir_theta,
		&gvar_mse,
		&gvar_min_pin_bounce,
		&gvar_max_bounce,
		&gvar_timing_left,
		&gvar_timing_right,
		&gvar_raymarche_step_size,
		&gvar_medium_pos,
		&gvar_medium_rot_y,
		&gvar_medium_scale,
		&gvar_env_map,
		&gvar_fixed_seed,
		&gvar_seed,
		&gvar_medium_xray_line_segments,
		&gvar_pin_sample_location,
		&gvar_continuous_path_sampling,
		//Screen Cursor
		&gvar_path_sampling_event,
		&gvar_screen_cursor_pos,
		&gvar_screen_cursor_enable,
		&gvar_screen_cursor_seed,
		// Cam
		&gvar_cam_fixpoint,
		&gvar_cam_distance,
		&gvar_cam_up,
		&gvar_cam_yaw,
		&gvar_cam_pitch,
		&gvar_cam_move_speed,
		&gvar_cam_turn_speed,
		&gvar_cam_scroll_speed,
		//Config
		&gvar_select_config,
		&gvar_save_config,
		&gvar_save_config_name,
		&gvar_reload_config
		// clang-format on
};

std::unordered_map<GVar, bool> gVarHasChanged;

ShaderConst sConst{};

// Models
ModelInfo cursor = {"arrow_cursor/arrow_cursor.obj", vec3(0.0, 0.0, 0.0), 0.05, vec3(0.0, 0.0, 0.0)};
ModelInfo arrow = {"arrow/arrow.obj", vec3(0.0, 0.0, 0.0), 1.0, vec3(0.0, 0.0, 0.0)};
ModelInfo cornellBox = {"cornell_box/cornell_box.obj", vec3(0,0.2,-0.3), 0.1, vec3(0.0,180.0,0.0)};
ModelInfo sponza = {"sponza/sponza_v2.obj", vec3(0.0,0.2,-0.3), 0.1, vec3(0.0,180.0,0.0)};
std::vector<ModelInfo> models     = {cornellBox, sponza};

uint32_t modelLoadFlags = MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE | MODEL_LOAD_FLAG_IS_OPAQUE | MODEL_LOAD_FLAG_COPYABLE;

// Medium Strategies
PerlinVolume              perlinVolume         = PerlinVolume();
UniformPinGenerator       pinGenerator         = UniformPinGenerator();
ArrayTransmittanceEncoder transmittanceEncoder = ArrayTransmittanceEncoder();
PinGridGenerator          pinGridGenerator     = PinGridGenerator();
// Path Trace Strategies
ReferencePathTracer 	 referencePathTracer  = ReferencePathTracer();
PinPathTracer 			 pinPathTracer 		 = PinPathTracer();

enum ViewType
{
	VIEW_TYPE_SPLIT,
	VIEW_TYPE_DIFF
};



int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();

	for( auto gv : gVars)
	{
		gVarHasChanged[*gv] = true;
	}


	// Medium
	Medium          medium          = Medium();
	Buffer      mediumInstanceBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	PinStateManager pinStateManager = PinStateManager(&medium);

	// Scene
	USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance>();
	USceneData scene{};
	ModelInfo  model;


	// Persistent Resources:
	// HDR Images for path tracing
	Image img_debug = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, viewDimensions.width, viewDimensions.height);
	
	Buffer plotBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, MAX_PLOT_POINTS * MAX_PLOTS * sizeof(float));
	guiConf.showPlots = true;
	guiConf.plot1 = createSwapchainAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, bottomGuiDimensions.width * 0.95, bottomGuiDimensions.height * 0.45);
	guiConf.plot2 = createSwapchainAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, bottomGuiDimensions.width * 0.95, bottomGuiDimensions.height * 0.45);
	
	// Path Tracer
	uint32_t maxLineSegmentCount       = gvar_max_bounce.set.range.max.v_uint * LINE_SEGMENTS_PER_BOUNCE;
	Buffer   lineSegmentInstanceBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, maxLineSegmentCount * sizeof(GLSLInstance));
	ComparativePathTracer pathTracer                = ComparativePathTracer(viewDimensions.width, viewDimensions.height, lineSegmentInstanceBuffer, plotBuffer, maxLineSegmentCount);
	Buffer mseBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(float));

	// Uniform Buffers
	sConst.alloc();

	// Load static models
	CmdBuffer cmdBuf = createCmdBuffer(gState.heap);
	ModelData cursorModel = gState.modelCache->fetch<GLSLVertex>(cmdBuf, cursor.path, 0);
	cmdFillBuffer(cmdBuf, lineSegmentInstanceBuffer, 0, maxLineSegmentCount * sizeof(GLSLInstance), 0);
	cmdFillBuffer(cmdBuf, plotBuffer, 0, plotBuffer->getSize(), 0);
	executeImmediat(cmdBuf);

	FixedCamera cam = FixedCamera(loadCamState());
	EventManager em = EventManager();
	em.init(&cam);

	gState.updateSwapchainAttachments();
	// Main Loop
	while(!gState.io.shouldTerminate())
	{
		em.newFrame();
		

		if (em.requestModelLoad())        // Load Scene
		{
			scene.garbageCollect();
			model              = models[gvar_model.val.v_uint];
			CmdBuffer cmdBuf = createCmdBuffer(gState.heap);

			// Solid Geometry
			sceneBuilder.reset();
			if (gvar_env_map.val.v_uint > 0)
			{
				std::string envmap = "/envmap/2k/" + gvar_env_map.set.list[gvar_env_map.val.v_uint];
				sceneBuilder.loadEnvMap(envmap, glm::uvec2(64, 64));
			}

			GLSLInstance instance{};
			instance.cullMask = 0xFF;
			instance.mat      = model.getObjToWorldMatrix();
			instance.color    = glm::vec3(0.0);
			sceneBuilder.addModel(cmdBuf, model.path, &instance, 1);
			sceneBuilder.addModel(cmdBuf, arrow.path, lineSegmentInstanceBuffer, maxLineSegmentCount);
			scene = sceneBuilder.create(cmdBuf, gState.heap, SCENE_LOAD_FLAG_ALLOW_RASTERIZATION);
			executeImmediat(cmdBuf);

		}
		CmdBuffer  cmdBuf       = createCmdBuffer(gState.frame->stack);

		getCmdPlot(plotBuffer, guiConf.plot1, 0, vec3(1.0, 0.0, 0.0)).exec(cmdBuf);
		getCmdPlot(plotBuffer, guiConf.plot2, MAX_PLOT_POINTS, vec3(0.0, 0.0, 1.0)).exec(cmdBuf);
		cmdTransitionLayout(cmdBuf, guiConf.plot1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cmdTransitionLayout(cmdBuf, guiConf.plot2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


		Image      swapchainImg = getSwapchainImage();
		// Update instance data and tlas
		scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.frame->stack));
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT , VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
		// Update medium
		GLSLMediumInstance mediumInstance{};
		mediumInstance.mat = glm::mat4(1.0f);
		mediumInstance.mat = glm::rotate(mediumInstance.mat, glm::radians(gvar_medium_rot_y.val.v_float), glm::vec3(0.0f, 0.0f, 1.0f));
		mediumInstance.mat = glm::scale(mediumInstance.mat, glm::vec3(gvar_medium_scale.val.v_float));
		mediumInstance.mat = glm::translate(mediumInstance.mat, gvar_medium_pos.val.getVec3());

		mediumInstance.invMat = glm::inverse(mediumInstance.mat);
		mediumInstance.albedo = gvar_medium_albedo.val.getVec3();
		cmdWriteCopy(cmdBuf, mediumInstanceBuffer, &mediumInstance, sizeof(GLSLMediumInstance));
		MediumBuildInfo buildInfo{};
		buildInfo.volGenerator = &perlinVolume;
		buildInfo.pinGenerator = &pinGenerator;
		buildInfo.transmittanceEncoder = &transmittanceEncoder;
		buildInfo.pinGridGenerator = &pinGridGenerator;
		MediumBuildTasks buildTasks = medium.update(cmdBuf, buildInfo);
		if (buildTasks.buildPinGrid || pinStateManager.requiresUpdate())
		{
			pinStateManager.update(cmdBuf);
		}
		// Reset accumulation
		if (em.ptReset)
		{
			pathTracer.reset(cmdBuf, &referencePathTracer, &pinPathTracer);
		}

		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		// Path tracing
		VkRect2D_OP    viewRect              = getScissorRect(viewDimensions);
		if (!em.debugView)
		{
			RenderInfo renderInfo           = RenderInfo();
			renderInfo.frameIdx             = em.frameCounter;
			renderInfo.pCamera              = &cam;
			renderInfo.pMediun              = &medium;
			renderInfo.mediumInstanceBuffer  = mediumInstanceBuffer;
			renderInfo.pSceneData            = &scene;
			renderInfo.cursorPos.x           = gvar_screen_cursor_pos.val.getVec4().x;
			renderInfo.cursorPos.y           = gvar_screen_cursor_pos.val.getVec4().y;
			pathTracer.render(cmdBuf, renderInfo);
			switch (em.viewType)
			{
				case VIEW_TYPE_SPLIT:
					pathTracer.showSplitView(cmdBuf, swapchainImg, em.ptSplittCoef, viewRect);
					break;
				case VIEW_TYPE_DIFF:
					pathTracer.showDiff(cmdBuf, swapchainImg, viewRect);
					break;
				default:
					break;
			}
			// Compute MSE
			gvar_mse.val.v_float = 1000.0*pathTracer.computeMSE(cmdBuf);
		}
		// Rasterization for debugging
		else
		{
			img_debug->setClearValue(ClearValue(0.0f, 0.0f, 0.0f, 1.0f));
			ModelData sceneModel = gState.modelCache->fetch<GLSLVertex>(cmdBuf, model.path, modelLoadFlags);
			cmdShowTriangles<GLSLVertex>(cmdBuf, gState.frame->stack, img_debug, sceneModel.vertexBuffer, sceneModel.indexBuffer, &cam, model.getObjToWorldMatrix(), true);
			cmdShowBoxFrame(cmdBuf, gState.frame->stack, img_debug, &cam, mediumInstance.mat, false, vec4(0.0, 0.0, 1.0, 1.0));
			glm::mat4 cursorMatrix = glm::translate(glm::mat4(1.0), gvar_cursor_pos.val.getVec3());
			cursorMatrix = glm::rotate(cursorMatrix, gvar_cursor_dir_theta.val.v_float, vec3(0.0, 0.0, 1.0));
			cursorMatrix = glm::rotate(cursorMatrix, gvar_cursor_dir_phi.val.v_float, vec3(0.0, 1.0, 0.0));
			cursorMatrix           = mediumInstance.mat * cursorMatrix * cursor.getObjToWorldMatrix();
			cmdShowAlbedo(cmdBuf, gState.frame->stack, img_debug, cursorModel, &cam, cursorMatrix, false);
			cmdVisualizePins(cmdBuf, gState.frame->stack, img_debug, medium.pins, pinStateManager.pinState, &cam, mediumInstance.mat, false, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			getCmdAdvancedCopy(img_debug, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			                   VkRect2D_OP(img_debug->getExtent2D()), viewRect)
			    .exec(cmdBuf);
		}
		// Add gui
		
		Rect2D<float> leftGuiBody = {viewMargin, viewDimensions.y, viewDimensions.x - 2.0 * viewMargin, 1.0 - viewMargin - viewDimensions.y};
		getCmdDrawRect(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, glm::vec4(0.45,0.45,0.5,1.0), getScissorRect(leftGuiBody)).exec(cmdBuf);
		cmdRenderGui(cmdBuf, swapchainImg);

		swapBuffers({cmdBuf});
	}
	storeGVar(gVars, configPath + "gvar.json");
	vkDeviceWaitIdle(gState.device.logical);
	pathTracer.destroy();
	gState.destroy();
}
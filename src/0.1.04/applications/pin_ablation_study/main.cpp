#include "config.h"
#include "pt_interface.h"
#include "ui.h"
#include <random>
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders/";
using namespace glm;

// Scene
GVar gvar_emission_scale_al{"Area light emission scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE, {0.0f, 100000.f}};
GVar gvar_emission_scale_env_map{"Env map emission scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE, {0.0f, 10.f}};
GVar gvar_skip_geometry{"Skip geometry", false, GVAR_BOOL, GUI_CAT_SCENE};

// Medium
GVar gvar_medium_density_scale{"Medium density scale", 1000.f, GVAR_FLOAT_RANGE, GUI_CAT_MEDIUM, {0.f, 4000.f}};

// Path Tracing
GVar gvar_ray_march_step_size{"Ray March Step Size", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PATH_TRACING, {0.01f, 1.f}};
GVar gvar_bounce_count{"Bounce Count", 5U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {1U, 16U}};
GVar gvar_min_bounce{"Min Bounce", 0U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {0U, 16U}};
GVar gvar_fixed_seed{"Fixed seed", 0U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {0U, 10000U}};
GVar gvar_first_random_bounce{"First random bounce", 0U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {0U, 16U}};

// Tone Mapping
GVar gvar_tone_mapping_enable{"Tone mapping", true, GVAR_BOOL, GUI_CAT_TONE_MAPPING};
GVar gvar_tone_mapping_whitepoint{"Tone mapping whitepoint", 4.f, GVAR_FLOAT_RANGE, GUI_CAT_TONE_MAPPING, {1.f, 10.f}};
GVar gvar_tone_mapping_exposure{"Tone mapping exposure", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_TONE_MAPPING, {0.001f, 1.f}};

// Pins
GVar gvar_pin_pos_grid_size{"Pin Pos Grid Size", 10U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 64U}};
GVar gvar_pin_dir_grid_size{"Pin Dir Grid Size", 8U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 256U}};
GVar gvar_pin_ray_march_step_size_coefficient{"Pin ray march step size coefficient", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.01f, 1.f}};
GVar gvar_pin_write_pin_step_size{"Pin write step size", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.01f, 1.f}};
GVar gvar_pin_update_mode{"Pin update mode", 1U, GVAR_ENUM, GUI_CAT_PINS, std::vector<std::string>({"All", "Trace"})};
GVar gvar_pin_update_rate{"Pin update rate", 1U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 100U}};
GVar gvar_pin_trace_update_ray_count{"Pin trace update ray count", 1000U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 1000000U}};
GVar gvar_force_rm_distance{"Force distance ray marching", 0U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 16U}};
GVar gvar_force_rm_transmittance_al{"Force transmittance ray marching for area lights", 0U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 16U}};
GVar gvar_force_rm_transmittance_env_map{"Force transmittance ray marching for env map", 0U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 16U}};
GVar gvar_pin_bit_mask_iterations{"Pin sample mask iterations", 5U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 10U}};
GVar gvar_jitter_pos{"Jitter pos", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.0f, 1.0f}};
GVar gvar_jitter_dir{"Jitter dir", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.0f, 1.0f}};

// Render Mode
GVar gvar_pin_mode_left{"Pin Mode Left", 0U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"None", "V1", "V2", "V3"})};
GVar gvar_sample_mode_left{"Sample Mode Left", 0U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"Unquantised", "Quantised", "Precomputed"})};
GVar gvar_pin_mode_right{"Pin Mode Right", 3U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"None", "V1", "V2", "V3"})};
GVar gvar_sample_mode_right{"Sample Mode Right", 2U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"Unquantised", "Quantised", "Precomputed"})};

// Metrics
GVar gvar_mse{"MSE: %.8f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_left{"Timing Left: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_right{"Timing Right: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};

// Debug
GVar gvar_enable_debuging{"Enable Debuging", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_total_contribution{"Write total contribution", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_indirect_dir{"Write indirect direction", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_indirect_t{"Write indirect t", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_indirect_weight{"Write indirect weight", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_bounce{"Select Bounce", 0, GVAR_UINT_RANGE, GUI_CAT_DEBUG, {0, 5}};

int main()
{
	//// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();
	//// Load settings
	GVar::loadAll(configPath + "last_session.json");
	//// Image Estimator Comparator
	ImageEstimatorComparator iec = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, viewDimensions.width, viewDimensions.height);
	//// Init other stuff
	FixedCamera                                           cam          = FixedCamera(DefaultFixedCameraState());
	USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance>();
	USceneData scene;
	//// Load stuff:
	CmdBuffer cmdBuf = createCmdBuffer(gState.frame->stack);
	//// Load Geometry
	//sceneBuilder.loadEnvMap("/envmap/2k/autumn_field_2k.hdr", glm::uvec2(64, 64));
	sceneBuilder.loadEnvMap("/envmap/2k/cloudy_dusky_sky_dome_2k.hdr", glm::uvec2(64, 64));
	//sceneBuilder.loadEnvMap("/envmap/2k/hochsal_field_2k.hdr", glm::uvec2(64, 64));
#ifdef RAY_TRACING_SUPPORT
	GLSLInstance instance{};
	instance.cullMask = 0xFF;
	instance.mat      = getMatrix(vec3(0, 0.2, -0.3), vec3(0.0, 180.0, 0.0), 0.1);
	sceneBuilder.addModel(cmdBuf, "under_the_c/scene_3.obj", &instance, 1);
	scene = sceneBuilder.create(cmdBuf, gState.heap);
	scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.heap));
#endif
	//// Create medium texture
	const uint32_t mediumExtent1D = 302;
	VkExtent3D     mediumExtent{mediumExtent1D, mediumExtent1D, mediumExtent1D};
	Image          medium = createImage(gState.heap, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, mediumExtent);
	cmdTransitionLayout(cmdBuf, medium, VK_IMAGE_LAYOUT_GENERAL);
	//// Create medium instances
	GLSLInstance mediumInstance{};
	mediumInstance.mat = getMatrix(vec3(-0.2, -0.2, -0.2), vec3(0, 0, 0), 0.4);
	mediumInstance.invMat = glm::inverse(mediumInstance.mat);
	mediumInstance.color  = vec3(1.0, 1.0, 1.0);
	mediumInstance.cullMask = 0xFF;
	Buffer mediumInstanceBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	uint32_t                              mediumInstanceCount = 50;
	std::vector<GLSLInstance>			  mediumInstances(mediumInstanceCount);
	std::mt19937                          rngGen(42);
	std::uniform_real_distribution<float> dist(0.0, 1.0);
	for (uint32_t i = 0; i < mediumInstanceCount; i++)
	{
		mediumInstances[i]        = mediumInstance;
		vec3  randomPos           = vec3(3.0)*(vec3(dist(rngGen), dist(rngGen), dist(rngGen)) - vec3(0.5));
		randomPos.y               = 0.2;
		float randomScale         = dist(rngGen) * 0.8 + 0.1;
		mediumInstances[i].mat    = getMatrix(randomPos, vec3(0, 0, -90), randomScale);
		mediumInstances[i].invMat = glm::inverse(mediumInstances[i].mat);
	}
	cmdWriteCopy(cmdBuf, mediumInstanceBuffer, mediumInstances.data(), mediumInstances.size() * sizeof(GLSLInstance));
	BLAS boxBlas = cmdBuildBoxBlas(cmdBuf, gState.heap);
	TLAS boxTlas = createTopLevelAS(gState.heap, mediumInstanceCount);
	default_scene::cmdBuildBoxIntersector<GLSLInstance>(cmdBuf, boxBlas, mediumInstanceBuffer, mediumInstanceCount, boxTlas);
	executeImmediat(cmdBuf);
	gState.updateSwapchainAttachments();

	//// Main Loop
	uint32_t      frameCount    = 0;
	vec2          lastClickPos  = vec2(1.0);
	float         splitViewCoef = 0.5;
	uint32_t      executionCounterLeft, executionCounterRight;
	TraceArgs     commonTraceArgs{};
	PinType       pinTypeLeft, pinTypeRight;
	PinSampleMode sampleModeLeft, sampleModeRight;
	Buffer        leftPinGridBuffer  = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	Buffer        rightPinGridBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	while (!gState.io.shouldTerminate())
	{
		//// Get Updates
		bool              firstFrame         = frameCount == 0;
		bool              shaderRecompile    = gState.io.keyPressedEvent[GLFW_KEY_R];
		bool              leftClickInView    = mouseInView(viewDimensions) && gState.io.mouse.leftPressedEvent();
		bool              leftPressedInView  = mouseInView(viewDimensions) && gState.io.mouse.leftPressed;
		std::vector<bool> settingsChanged    = buildGui();
		bool              anySettingsChanged = orOp(settingsChanged);
		bool              camRotated         = mouseInView(viewDimensions) && cam.keyControl(0.016);
		bool              camMoved           = mouseInView(viewDimensions) && gState.io.mouse.rightPressed && cam.mouseControl(0.016);
		bool              viewChange         = camRotated || camMoved || anySettingsChanged || firstFrame || shaderRecompile || gState.io.swapchainRecreated();
		bool              selectPixel        = leftClickInView && gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL];


		if (shaderRecompile)
		{
			clearShaderCache();
			gState.io.buildShaderLib();
		}

		if (selectPixel)
		{
			lastClickPos = mouseViewCoord(viewDimensions);
		}

		if (leftPressedInView)
		{
			splitViewCoef += gState.io.mouse.change.x/(float)getScissorRect(viewDimensions).extent.width;
			splitViewCoef = glm::clamp(splitViewCoef, 0.f, 1.f);
		}
		//// Process Updates
		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		//// Reset accumulation
		if (viewChange)
		{
			iec.cmdReset(cmdBuf);
			executionCounterLeft = 0;
			executionCounterRight = 0;
		}
		// Process volume data
		if (settingsChanged[GUI_CAT_MEDIUM] || shaderRecompile || firstFrame)
		{
			Buffer scalarBuf;
			gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, scalarFieldPath + "csafe_heptane_302x302x302_uint8.raw", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			getCmdLoadScalarField(scalarBuf, medium, gvar_medium_density_scale.val.v_float).exec(cmdBuf);
		}

		//// Run Path Tracing
		CameraCI camCI{};
		camCI.pos      = cam.getPosition();
		camCI.frontDir = cam.getViewDirection();
		camCI.upDir    = cam.getViewUpDirection();
		camCI.frameIdx = frameCount;
		camCI.extent   = getScissorRect(viewDimensions).extent;
		camCI.yFovDeg  = 60.0;
		camCI.zNear    = 0.1;
		camCI.zFar     = 100.0;

		commonTraceArgs.cameraCI               = camCI;
		commonTraceArgs.sceneData              = scene;
		commonTraceArgs.mediumInstanceBuffer   = mediumInstanceBuffer;
		commonTraceArgs.mediumTlas             = boxTlas;
		commonTraceArgs.mediumTexture          = medium;

		commonTraceArgs.areaLightEmissionScale = gvar_emission_scale_al.val.v_float;
		commonTraceArgs.envMapEmissionScale    = gvar_emission_scale_env_map.val.v_float;

		// require shader recompilation
		if (firstFrame || leftClickInView || shaderRecompile)
		{
			// general config
			commonTraceArgs.rayMarchStepSize                                 = gvar_ray_march_step_size.val.v_float;
			commonTraceArgs.sampleCount                                      = 1;
			commonTraceArgs.maxDepth                                         = gvar_bounce_count.val.v_uint;
			commonTraceArgs.minDepth                                         = gvar_min_bounce.val.v_uint;
			commonTraceArgs.fixedSeed                                        = gvar_fixed_seed.val.v_uint;
			commonTraceArgs.firstRandomBounce                                = gvar_first_random_bounce.val.v_uint;
			commonTraceArgs.skipGeometry                                     = gvar_skip_geometry.val.v_bool;
			commonTraceArgs.force_ray_marched_distance_sampling              = gvar_force_rm_distance.val.v_uint;
			commonTraceArgs.force_ray_marched_transmittance_sampling_al      = gvar_force_rm_transmittance_al.val.v_uint;
			commonTraceArgs.force_ray_marched_transmittance_sampling_env_map = gvar_force_rm_transmittance_env_map.val.v_uint;

			// pin config
			commonTraceArgs.cvsArgs.defaultUpdateMode                = static_cast<CVSUpdateMode>(gvar_pin_update_mode.val.v_uint);
			commonTraceArgs.cvsArgs.updateRate                       = gvar_pin_update_rate.val.v_uint;
			commonTraceArgs.cvsArgs.traceUpdateArgs.rayCount         = gvar_pin_trace_update_ray_count.val.v_uint;
			commonTraceArgs.cvsArgs.traceUpdateArgs.writePinStepSize = gvar_pin_write_pin_step_size.val.v_float;
			commonTraceArgs.cvsArgs.rayMarchingCoefficient           = gvar_pin_ray_march_step_size_coefficient.val.v_float;
			commonTraceArgs.cvsArgs.pinGridExtent.positionGridSize   = gvar_pin_pos_grid_size.val.v_uint;
			commonTraceArgs.cvsArgs.pinGridExtent.directionGridSize  = gvar_pin_dir_grid_size.val.v_uint;
			commonTraceArgs.cvsArgs.pinArgs.bitMaskIterations        = gvar_pin_bit_mask_iterations.val.v_uint;
			commonTraceArgs.cvsArgs.pinArgs.jitterPos                = gvar_jitter_pos.val.v_float;
			commonTraceArgs.cvsArgs.pinArgs.jitterDir                = gvar_jitter_dir.val.v_float;
		}

		// Debugging visualizations
		commonTraceArgs.enableDebugging = gvar_enable_debuging.val.v_bool;
		if (commonTraceArgs.enableDebugging)
		{
			commonTraceArgs.debugArgs.pixelPos        = lastClickPos;
			commonTraceArgs.debugArgs.enableHistogram = true;
			commonTraceArgs.debugArgs.enablePtPlot    = true;

			commonTraceArgs.debugArgs.ptPlotOptions.writeTotalContribution = gvar_pt_plot_write_total_contribution.val.v_bool;
			commonTraceArgs.debugArgs.ptPlotOptions.writeIndirectDir       = gvar_pt_plot_write_indirect_dir.val.v_bool;
			commonTraceArgs.debugArgs.ptPlotOptions.writeIndirectT         = gvar_pt_plot_write_indirect_t.val.v_bool;
			commonTraceArgs.debugArgs.ptPlotOptions.writeIndirectWeight    = gvar_pt_plot_write_indirect_weight.val.v_bool;
			commonTraceArgs.debugArgs.ptPlotOptions.bounce                 = gvar_pt_plot_bounce.val.v_uint;

			if (viewChange || selectPixel)
			{
				commonTraceArgs.debugArgs.resetHistogram    = true;
				commonTraceArgs.debugArgs.maxHistCount      = maxHistogramCount;
				commonTraceArgs.debugArgs.maxHistValueCount = maxHistValueCount;
			}
		}

		if (viewChange || firstFrame || shaderRecompile)
		{
			pinTypeLeft     = static_cast<PinType>(gvar_pin_mode_left.val.v_uint);
			pinTypeRight    = static_cast<PinType>(gvar_pin_mode_right.val.v_uint);
			sampleModeLeft  = static_cast<PinSampleMode>(gvar_sample_mode_left.val.v_uint);
			sampleModeRight = static_cast<PinSampleMode>(gvar_sample_mode_right.val.v_uint);
		}

		TraceArgs traceArgsLeft                  = commonTraceArgs;
		traceArgsLeft.cvsArgs.pinGridBuffer      = leftPinGridBuffer;
		traceArgsLeft.pExecutionCounter          = &executionCounterLeft;
		traceArgsLeft.cvsArgs.pinArgs.type       = pinTypeLeft;
		traceArgsLeft.cvsArgs.pinArgs.sampleMode = sampleModeLeft;

		TraceArgs traceArgsRight                  = commonTraceArgs;
		traceArgsRight.cvsArgs.pinGridBuffer      = rightPinGridBuffer;
		traceArgsRight.pExecutionCounter          = &executionCounterRight;
		traceArgsRight.cvsArgs.pinArgs.type       = pinTypeRight;
		traceArgsRight.cvsArgs.pinArgs.sampleMode = sampleModeRight;

		iec.cmdRunEqualTime<TraceArgs>(cmdBuf, cmdTrace, traceArgsLeft, traceArgsRight, &gvar_timing_left.val.v_float, &gvar_timing_right.val.v_float);
		//// Show results
		gvar_mse.val.v_float = iec.getMSE();
		Image swapchainImg = getSwapchainImage();
		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);
		IECToneMappingArgs toneMappingArgs{};
		toneMappingArgs.useTonemapping = gvar_tone_mapping_enable.val.v_bool;
		toneMappingArgs.whitePoint     = gvar_tone_mapping_whitepoint.val.v_float;
		toneMappingArgs.exposure       = gvar_tone_mapping_exposure.val.v_float;
		iec.showSplitView(cmdBuf, swapchainImg, splitViewCoef, getScissorRect(viewDimensions), toneMappingArgs);
		cmdRenderGui(cmdBuf, swapchainImg);

		swapBuffers({cmdBuf});
		frameCount++;
	}
	//// Save settings
	GVar::storeAll(configPath + "last_session.json");
	gState.destroy();
}
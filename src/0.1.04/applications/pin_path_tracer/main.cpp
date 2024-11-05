#include "config.h"
#include "shader_interface.h"
#include "ui.h"
#include <random>
#include <random>
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders/";
using namespace glm;

extern GVar gvar_pt_plot_write_total_contribution;
extern GVar gvar_pt_plot_write_indirect_dir;
extern GVar gvar_pt_plot_write_indirect_t;
extern GVar gvar_pt_plot_write_indirect_weight;

extern GVar gvar_perlin_frequency;
extern GVar gvar_perlin_scale;
extern GVar gvar_pt_plot_bounce;

extern GVar gvar_mse;
extern GVar gvar_timing_left;
extern GVar gvar_timing_right;

extern GVar gvar_pin_pos_grid_size;
extern GVar gvar_pin_dir_grid_size;
extern GVar gvar_pin_ray_march_step_size;
extern GVar gvar_pin_write_pin_step_size;
extern GVar gvar_pin_update_rate;
extern GVar gvar_pin_update_all;


extern GVar gvar_ray_march_step_size;
extern GVar gvar_bounce_count;
extern GVar gvar_min_bounce;
extern GVar gvar_skip_geometry;

extern GVar gvar_tone_mapping_enable;
extern GVar gvar_tone_mapping_whitepoint;
extern GVar gvar_tone_mapping_exposure;
extern GVar gvar_pin_disable_bit_mask_sampling;
extern GVar gvar_pin_bit_mask_size;

extern GVar gvar_fixed_seed;
extern GVar gvar_first_random_bounce;

extern GVar gvar_enable_debuging;
extern GVar gvar_emission_scale_al;
extern GVar gvar_emission_scale_env_map;

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
	//sceneBuilder.addModel(cmdBuf, "cornell_box/cornell_box.obj", &instance, 1);
	sceneBuilder.addModel(cmdBuf, "under_the_c/scene_1.obj", &instance, 1);
	scene = sceneBuilder.create(cmdBuf, gState.heap);
	scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.heap));
#endif
	//// Load Medium
	PerlinNoiseArgs perlinArgs{};
	perlinArgs.scale              = gvar_perlin_scale.val.v_float;
	perlinArgs.min           = 0.0;
	perlinArgs.max           = 100.0;
	perlinArgs.frequency          = gvar_perlin_frequency.val.v_float;
	perlinArgs.falloffAtEdge = true;
	const uint32_t mediumExtent1D = 302;
	VkExtent3D     mediumExtent{mediumExtent1D, mediumExtent1D, mediumExtent1D};
	//VkExtent3D mediumExtent{128, 256, 256};
	//VkExtent3D mediumExtent{512, 512, 361};
	Image          medium = createImage(gState.heap, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, mediumExtent);
	cmdTransitionLayout(cmdBuf, medium, VK_IMAGE_LAYOUT_GENERAL);

	// Medium instances
	GLSLMediumInstance mediumInstance{};
	mediumInstance.mat = getMatrix(vec3(-0.2, -0.2, -0.2), vec3(0, 0, 0), 0.4);
	mediumInstance.invMat = glm::inverse(mediumInstance.mat);
	mediumInstance.albedo  = vec3(1.0, 1.0, 1.0);
	mediumInstance.cullMask = 0xFF;
	Buffer mediumInstanceBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	uint32_t                              mediumInstanceCount = 50;
	std::vector<GLSLMediumInstance>       mediumInstances(mediumInstanceCount);
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
	cmdWriteCopy(cmdBuf, mediumInstanceBuffer, mediumInstances.data(), mediumInstances.size() * sizeof(GLSLMediumInstance));



	BLAS boxBlas = cmdBuildBoxBlas(cmdBuf, gState.heap);
	TLAS boxTlas = createTopLevelAS(gState.heap, mediumInstanceCount);
	default_scene::cmdBuildBoxIntersector<GLSLMediumInstance>(cmdBuf, boxBlas, mediumInstanceBuffer, mediumInstanceCount, boxTlas);

	executeImmediat(cmdBuf);
	gState.updateSwapchainAttachments();
	//// Main Loop
	uint32_t frameCount = 0;
	vec2 lastClickPos = vec2(1.0);
	float splitViewCoef = 0.5;
	while (!gState.io.shouldTerminate())
	{
		//// Get Updates
		bool firstFrame = frameCount == 0;
		bool shaderRecompiled = false;
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
			gState.io.buildShaderLib();
			shaderRecompiled = true;
		}
		bool leftClickInView = false;
		if (mouseInView(viewDimensions) && gState.io.mouse.leftPressedEvent())
		{
			lastClickPos = mouseViewCoord(viewDimensions);
			leftClickInView = true;
		}
		bool viewHasChanged                  = mouseInView(viewDimensions) && cam.keyControl(0.016);
		viewHasChanged                       = (mouseInView(viewDimensions)  && gState.io.mouse.rightPressed && cam.mouseControl(0.016)) || viewHasChanged;
		viewHasChanged                       = viewHasChanged || gState.io.swapchainRecreated();
		std::vector<bool> settingsChanged    = buildGui();
		bool              anySettingsChanged = orOp(settingsChanged);
		if (mouseInView(viewDimensions) && gState.io.mouse.leftPressed)
		{
			splitViewCoef += gState.io.mouse.change.x/(float)getScissorRect(viewDimensions).extent.width;
			splitViewCoef = glm::clamp(splitViewCoef, 0.f, 1.f);
		}
		//// Process Updates
		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		//// Reset accumulation
		if (viewHasChanged || anySettingsChanged || shaderRecompiled || firstFrame)
		{
			iec.cmdReset(cmdBuf);
		}
		bool resetPlots = viewHasChanged || anySettingsChanged || shaderRecompiled || firstFrame;
		resetPlots      = resetPlots || (leftClickInView && gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL]);
		// Regenerate noise
		if (settingsChanged[GUI_CAT_NOISE] || shaderRecompiled || firstFrame)
		{
			/*perlinArgs.frequency = gvar_perlin_frequency.val.v_float;
			perlinArgs.scale     = gvar_perlin_scale.val.v_float;
			getCmdPerlinNoise(medium, perlinArgs).exec(cmdBuf);*/
			Buffer scalarBuf;
			//gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, scalarFieldPath + "csafe_heptane_302x302x302_uint8.raw", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			//gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, scalarFieldPath + "chameleon_1024x1024x1080_uint16.raw", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, scalarFieldPath + "csafe_heptane_302x302x302_uint8.raw", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			//gState.binaryLoadCache->fetch(cmdBuf, scalarBuf, scalarFieldPath + "bunny_512x512x361_uint16.raw", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			getCmdLoadScalarField(scalarBuf, medium, gvar_perlin_scale.val.v_float).exec(cmdBuf);
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

		TraceArgs traceArgs{};
		traceArgs.sampleCount               = 1;
		traceArgs.maxDepth                  = gvar_bounce_count.val.v_uint;
		traceArgs.rayMarchStepSize          = gvar_ray_march_step_size.val.v_float;
		traceArgs.cameraCI                  = camCI;
		traceArgs.sceneData                 = scene;
		traceArgs.mediumInstanceBuffer      = mediumInstanceBuffer;
		traceArgs.mediumTexture             = medium;
		traceArgs.enableDebugging           = gvar_enable_debuging.val.v_bool;
		traceArgs.mediumTlas                = boxTlas;
		traceArgs.envMapEmissionScale       = gvar_emission_scale_env_map.val.v_float;
		traceArgs.areaLightEmissionScale    = gvar_emission_scale_al.val.v_float;
		traceArgs.debugArgs.pixelPos        = lastClickPos;
		traceArgs.debugArgs.enableHistogram = true;
		traceArgs.debugArgs.enablePtPlot    = true;
		traceArgs.debugArgs.minDepth        = gvar_min_bounce.val.v_uint;
		traceArgs.debugArgs.skipGeometry    = gvar_skip_geometry.val.v_bool;

		traceArgs.debugArgs.ptPlotOptions.writeTotalContribution = gvar_pt_plot_write_total_contribution.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.writeIndirectDir       = gvar_pt_plot_write_indirect_dir.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.writeIndirectT         = gvar_pt_plot_write_indirect_t.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.writeIndirectWeight    = gvar_pt_plot_write_indirect_weight.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.bounce                 = gvar_pt_plot_bounce.val.v_uint;
		traceArgs.debugArgs.fixedSeed = gvar_fixed_seed.val.v_uint;
		traceArgs.debugArgs.firstRandomBounce = gvar_first_random_bounce.val.v_uint;

		if (leftClickInView)
		{
			traceArgs.debugArgs.enablePlot        = true;
			traceArgs.debugArgs.maxPlotCount      = maxPlotCount;
			traceArgs.debugArgs.maxPlotValueCount = maxPlotValueCount;
		}

		if (resetPlots)
		{
			traceArgs.debugArgs.resetHistogram = true;
			traceArgs.debugArgs.maxHistCount      = maxHistogramCount;
			traceArgs.debugArgs.maxHistValueCount = maxHistValueCount;
		}

		TraceArgs traceArgs2 = traceArgs;
		traceArgs2.debugArgs.histogramDataOffset = getLeftHistogramOffset();
		traceArgs2.enablePins = true;
		traceArgs2.pinArgs.posGridSize = gvar_pin_pos_grid_size.val.v_uint;
		traceArgs2.pinArgs.dirGridSize = gvar_pin_dir_grid_size.val.v_uint;
		traceArgs2.pinArgs.count = gvar_pin_update_rate.val.v_uint;
		traceArgs2.pinArgs.rayMarchStepSize = gvar_pin_ray_march_step_size.val.v_float;
		traceArgs2.pinArgs.writePinStepSize = gvar_pin_write_pin_step_size.val.v_float;
		traceArgs2.pinArgs.bitMaskIterations = gvar_bitmask_iterations.val.v_uint;
		traceArgs2.pinArgs.executionID       = frameCount;
		traceArgs2.pinArgs.disableBitmaskSampling = gvar_pin_disable_bit_mask_sampling.val.v_bool;
		traceArgs2.pinArgs.bitMaskSize = gvar_pin_bit_mask_size.val.v_uint;
		traceArgs2.pinArgs.updateAll = gvar_pin_update_all.val.v_bool;


		iec.cmdRunEqualTime<TraceArgs>(cmdBuf, cmdTrace, traceArgs, traceArgs2, &gvar_timing_left.val.v_float, &gvar_timing_right.val.v_float);
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
#include "config.h"
#include "pt_interface.h"
#include "ui.h"
#include <random>
#include "OfflineRenderer.h"
#include "SinglePassOfflineRenderer.h"
#include "misc.h"
#include "parse_args.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders/";
using namespace glm;


extern GVar gvar_medium_load;
extern GVar gvar_medium_inst_update;
extern GVar gvar_medium_instamce_shader_params;
extern GVar gvar_medium_instamce_shader_params_input;
extern GVar gvar_camera_reset;



GVar gvar_menu{"Menu", 0U, GVAR_ENUM, GUI_CAT_MENU_BAR, std::vector<std::string>({"File", "Scene", "Settings", "Evaluation", "Debug"}), GVAR_FLAGS_V2};

// Scene params
GVar gvar_emission_scale_al{"Area light emission scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_PARAMS, {0.0f, 1000.f}};
GVar gvar_emission_scale_env_map{"Env map emission scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_PARAMS, {0.0f, 10.f}};
GVar gvar_skip_geometry{"Skip geometry", false, GVAR_BOOL, GUI_CAT_SCENE_PARAMS};

GVar gvar_medium_density_scale{"Medium density scale", 1000.f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM_DENSITY, {0.f, 500.f}};
GVar gvar_medium_min_density{"Medium min density", 0.f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM_DENSITY, {0.f, 1.f}};
GVar gvar_medium_scattering_function {"Medium scattering function", 0U, GVAR_ENUM, GUI_CAT_SCENE_MEDIUM_DENSITY, std::vector<std::string>({"Isotropic", "HG"})};
GVar gvar_medium_scattering_function_g{"Medium scattering function g", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_SCENE_MEDIUM_DENSITY, {-1.f, 1.f}};

// Path Tracing
GVar gvar_ray_march_step_size{"RM Step Size", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PATH_TRACING, {0.01f, 1.f}};
GVar gvar_bounce_count{"Bounce Count", 5U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {1U, 16U}};
GVar gvar_min_bounce{"Min Bounce", 0U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {0U, 16U}};
GVar gvar_pt_seed{"Seed", 42U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {0U, 10000U}};
GVar gvar_first_random_bounce{"First random bounce", 0U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {0U, 16U}};
GVar gvar_sub_sample_mode{"Subsample mode", 1U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {1U, 4U}};

// Tone Mapping
GVar gvar_tone_mapping_enable{"Tone mapping", true, GVAR_BOOL, GUI_CAT_TONE_MAPPING};
GVar gvar_tone_mapping_whitepoint{"Tone mapping whitepoint", 4.f, GVAR_FLOAT_RANGE, GUI_CAT_TONE_MAPPING, {1.f, 10.f}};
GVar gvar_tone_mapping_exposure{"Tone mapping exposure", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_TONE_MAPPING, {0.001f, 1.f}};

// Pins
GVar gvar_pin_pos_grid_size{"Pin Pos Grid Size", 10U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 64U}};
GVar gvar_pin_dir_grid_size{"Pin Dir Grid Size", 8U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 256U}};
GVar gvar_pin_ray_march_step_size_coefficient{"Pin RM step size coefficient", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.01f, 1.f}};
GVar gvar_pin_write_pin_step_size{"Pin write step size", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.01f, 1.f}};
GVar gvar_pin_update_mode{"Pin update mode", 1U, GVAR_ENUM, GUI_CAT_PINS, std::vector<std::string>({"All", "Trace"})};
GVar gvar_pin_update_rate{"Pin update rate", 1U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 100U}};
GVar gvar_pin_trace_update_ray_count{"Pin trace update ray count", 1000U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 50000U}};
GVar gvar_force_rm_distance{"Force distance RM", 0U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 16U}};
GVar gvar_force_rm_transmittance_al{"Area lights force transmittance RM", 0U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 16U}};
GVar gvar_force_rm_transmittance_env_map{"Env map force transmittance RM", 0U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 16U}};
GVar gvar_pin_bit_mask_iterations{"Pin sample mask iterations", 5U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 10U}};
GVar gvar_jitter_pos{"Jitter pos", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.0f, 1.0f}};
GVar gvar_jitter_dir{"Jitter dir", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.0f, 1.0f}};

// Render Mode
GVar gvar_pin_mode_left{"Pin Mode Left", 0U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"None", "V1", "V2", "V3"})};
GVar gvar_sample_mode_left{"Sample Mode Left", 0U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"Unquantised", "Quantised", "Precomputed"})};
GVar gvar_pin_mode_right{"Pin Mode Right", 3U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"None", "V1", "V2", "V3"})};
GVar gvar_sample_mode_right{"Sample Mode Right", 2U, GVAR_ENUM, GUI_CAT_RENDER_MODE, std::vector<std::string>({"Unquantised", "Quantised", "Precomputed"})};

// Metrics
GVar gvar_mse{"Avg squared diff: %.8f", 0.f, GVAR_DISPLAY_FLOAT, GUI_CAT_METRICS};
GVar gvar_timing_left{"Timing Left: %.4f", 1.f, GVAR_DISPLAY_FLOAT, GUI_CAT_METRICS};
GVar gvar_timing_right{"Timing Right: %.4f", 1.f, GVAR_DISPLAY_FLOAT, GUI_CAT_METRICS};



// Evaluation
GVar gvar_eval_resolution{"Eval Resolution", 0U, GVAR_ENUM, GUI_CAT_EVALUATION_PARAMS, std::vector<std::string>({
	"1920x1080",
	"1280x720",
	"640x360",
	"320x180",
	"1024x1024",
	"512x512",
	"256x256",
	"128x128",
	})};

std::vector<VkExtent2D> cResolutions
{
	{1920, 1080},
	{1280, 720},
	{640, 360},
	{320, 180},
	{1024, 1024},
	{512, 512},
	{256, 256},
	{128, 128},
};
GVar gvar_sample_count{"Sample Count", 1U, GVAR_UINT_RANGE, GUI_CAT_EVALUATION_PARAMS, {1U, 64}};
GVar gvar_eval_ref_target_time{"Ref Target Time", 10.f, GVAR_FLOAT_RANGE, GUI_CAT_EVALUATION_PARAMS, {1.f, 300.f}};
GVar gvar_eval_comp_target_time{"Eval Target Time", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_EVALUATION_PARAMS, {1.f, 300.f}};
GVar gvar_eval_comp_target_invokations{"Eval Target Invokations", 1, GVAR_UINT_RANGE, GUI_CAT_EVALUATION_PARAMS, {1, 100}};
GVar gvar_eval_mode{"Eval Mode", 0U, GVAR_ENUM, GUI_CAT_EVALUATION_PARAMS, std::vector<std::string>({"Equal Time", "Equal Samples"})};

GVar gvar_eval_keep_realtime_render{"Keep realtime render", false, GVAR_BOOL, GUI_CAT_EVALUATION};
GVar gvar_eval_name{"Eval Name", std::string("default"), GVAR_TEXT_INPUT, GUI_CAT_EVALUATION};
GVar gvar_evaluate{"Evaluate", false, GVAR_EVENT, GUI_CAT_EVALUATION};
GVar gvar_remaning_tasks{"Remaining Tasks: %d", 0U, GVAR_DISPLAY_UINT, GUI_CAT_EVALUATION};
GVar gvar_task_progress{"Task Progress:", 0.f, GVAR_DISPLAY_UNORM, GUI_CAT_EVALUATION};

// Debug
GVar gvar_enable_debuging{"Enable Debuging", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_total_contribution{"Write total contribution", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_indirect_dir{"Write indirect direction", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_indirect_t{"Write indirect t", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_write_indirect_weight{"Write indirect weight", false, GVAR_BOOL, GUI_CAT_DEBUG};
GVar gvar_pt_plot_bounce{"Select Bounce", 0, GVAR_UINT_RANGE, GUI_CAT_DEBUG, {0, 5}};




int main(int argc, char *argv[])
{
	AppArgs appArgs{};
	bool offlineMode = false;
	if (parse_args(argc, argv, appArgs))
	{
		loadArgs(appArgs);
		offlineMode = true;
	}
	else
	{
		GVar::loadAll(configPath + "last_session.json");
	}


	//// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI ioCI;
	if (offlineMode)
	{
		ioCI = DefaultIOControlerCI(APP_NAME, 100, 10, false);
	}
	else
	{
		ioCI = DefaultIOControlerCI(APP_NAME, 1000, 700, true);
	}
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();
	initImFile();
	//// Load settings
	//// Image Estimator Comparator
	ImageEstimatorComparator iec = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, viewDimensions.width, viewDimensions.height);
	//// Init other stuff
	FixedCamera    cam = FixedCamera(DefaultFixedCameraState());
	TraceResources traceResources{};
	gState.updateSwapchainAttachments();

	//// Main Loop
	uint32_t      frameCount    = 0;
	vec2          lastClickPos  = vec2(1.0);
	float         splitViewCoef = 0.5;

	int terminateOnFrame = -1;

	ResourceCache traceResourceCache = ResourceCache();
	ResourceCache mediumInstResCache = ResourceCache();
	uint32_t      executionCounterLeft, executionCounterRight;
	TraceArgs     traceArgsLeft      = TraceArgs(&traceResourceCache, gState.heap, &executionCounterLeft);
	TraceArgs     traceArgsRight     = TraceArgs(&traceResourceCache, gState.heap, &executionCounterRight);
	SIOfflineRenderer offlineRenderer    = SIOfflineRenderer();

	while (!gState.io.shouldTerminate())
	{
		if (processLoadStoreFile())
		{
			frameCount = 0;
		}
		processTraceParams();
		CmdBuffer cmdBuf = createCmdBuffer(gState.frame->stack);
		//// Get Updates
		cam                          = FixedCamera(loadCamState());
		bool viewFocus               = !GVar::holdsFocus() && mouseInView(viewDimensions);
		bool firstFrame              = frameCount == 0;
		bool secondFrame             = frameCount == 1;
		bool fullShaderRecompile     = viewFocus && gState.io.keyPressedEvent[GLFW_KEY_R];
		bool fastShaderRecompile     = viewFocus && gState.io.keyPressedEvent[GLFW_KEY_F];
		bool shaderRecompile         = fullShaderRecompile || fastShaderRecompile;
		bool leftClickInView         = viewFocus && gState.io.mouse.leftPressedEvent();
		bool leftPressedInView       = viewFocus && gState.io.mouse.leftPressed;
		bool camRotated              = viewFocus && cam.keyControl();
		bool camMoved                = viewFocus && gState.io.mouse.rightPressed && cam.mouseControl();
		bool selectPixel             = leftClickInView && gState.io.keyPressed[GLFW_KEY_LEFT_CONTROL];
		gvar_camera_reset.val.v_bool = viewFocus && gState.io.keyPressedEvent[GLFW_KEY_T];
		saveCamState(cam.getState());

		if (fullShaderRecompile)
		{
			clearShaderCache();
			traceResourceCache.clearShaders();
			mediumInstResCache.clearShaders();
			gState.shaderLog = "";
			gState.io.buildShaderLib();
		}
		if (fastShaderRecompile)
		{
			vkDeviceWaitIdle(gState.device.logical);
			traceResourceCache.clearShaders();
			gState.shaderLog = "";
			gState.io.buildShaderLib();
		}

		std::vector<bool> settingsChanged     = buildGui(cmdBuf, &iec);

		if (gvar_medium_inst_update.val.v_bool)
		{
			gvar_medium_instamce_shader_params.val.v_char_array = gvar_medium_instamce_shader_params_input.val.v_char_array;
		}

		if (offlineMode)
		{
			gvar_eval_keep_realtime_render.val.v_bool = false;
			gvar_evaluate.val.v_bool                  = firstFrame;
			if (!gvar_evaluate.val.v_bool && offlineRenderer.getTaskQueueSize() == 0 && terminateOnFrame ==-1)
			{
				terminateOnFrame = frameCount + 6;
			}
		}

		if (terminateOnFrame >= 0 && frameCount >= terminateOnFrame)
		{
			gState.io.requestTerminate();
		}

		if (gvar_medium_inst_update.val.v_bool)
		{
			vkDeviceWaitIdle(gState.device.logical);
			mediumInstResCache.clearShaders();
			gState.shaderLog = "";
			gState.io.buildShaderLib();
		}

		auto              guiCatChanged = [settingsChanged](uint32_t setting) -> bool {
            return gvar_menu.val.v_uint == setting >> GUI_CAT_SHIFT && settingsChanged[setting & GUI_INDEX_MASK];
		};
		// clang-format off
		bool    renderSettingsChanged = 
			guiCatChanged(GUI_CAT_SCENE_GENERAL)
			|| guiCatChanged(GUI_CAT_SCENE_MEDIUM_DENSITY)
			|| gvar_medium_load.val.v_bool
			|| gvar_medium_inst_update.val.v_bool
			|| guiCatChanged(GUI_CAT_SCENE_MEDIUM_INSTANCE_ARGS)
			|| guiCatChanged(GUI_CAT_SCENE_TRANSFORMS)
			|| guiCatChanged(GUI_CAT_SCENE_PARAMS)
			|| guiCatChanged(GUI_CAT_PATH_TRACING)
			|| guiCatChanged(GUI_CAT_TONE_MAPPING)
			|| guiCatChanged(GUI_CAT_PINS)
			|| guiCatChanged(GUI_CAT_RENDER_MODE);
			// clang-format on
		bool anySettingsChanged = orOp(settingsChanged);
		bool viewChange         = camRotated || camMoved || renderSettingsChanged || firstFrame || secondFrame || shaderRecompile || gState.io.swapchainRecreated();

		if (selectPixel)
		{
			lastClickPos = mouseViewCoord(viewDimensions);
		}
		if (leftPressedInView)
		{
			splitViewCoef += gState.io.mouse.change.x/(float)getScissorRect(viewDimensions).extent.width;
			splitViewCoef = glm::clamp(splitViewCoef, 0.f, 1.f);
		}
		//// Reset accumulation
		if (viewChange)
		{
			iec.cmdReset(cmdBuf);
			executionCounterLeft = 0;
			executionCounterRight = 0;
		}
		if (firstFrame)
		{
			traceResources.cmdLoadAll(cmdBuf, gState.heap, &mediumInstResCache);
		}
		else
		{
			traceResources.cmdLoadUpdate(cmdBuf, gState.heap, &mediumInstResCache, settingsChanged);
		}

		CameraCI camCI{};
		camCI.pos      = cam.getPosition();
		camCI.frontDir = cam.getViewDirection();
		camCI.upDir    = cam.getViewUpDirection();
		camCI.frameIdx = frameCount;
		camCI.extent   = getScissorRect(viewDimensions).extent;
		camCI.yFovDeg  = 60.0;
		camCI.zNear    = 0.1;
		camCI.zFar     = 100.0;

		TraceSceneParams sceneParams{};
		sceneParams.areaLightEmissionScale = gvar_emission_scale_al.val.v_float;
		sceneParams.envMapEmissionScale    = gvar_emission_scale_env_map.val.v_float;
		sceneParams.cameraCI               = camCI;
		sceneParams.skipGeometry           = gvar_skip_geometry.val.v_bool;
		sceneParams.densityScale		   = gvar_medium_density_scale.val.v_float;
		sceneParams.minDensity			   = gvar_medium_min_density.val.v_float;
		sceneParams.scatterFunc            = gvar_medium_scattering_function.val.v_uint;
		sceneParams.scatterFuncG           = gvar_medium_scattering_function_g.val.v_float;

		TracerConfig tracerConfig{};
		tracerConfig.rayMarchStepSize                                 = gvar_ray_march_step_size.val.v_float;
		tracerConfig.minDepth                                         = gvar_min_bounce.val.v_uint;
		tracerConfig.seed                                             = gvar_pt_seed.val.v_uint;
		tracerConfig.firstRandomBounce                                = gvar_first_random_bounce.val.v_uint;
		tracerConfig.subSampleMode                                    = gvar_sub_sample_mode.val.v_uint;
		tracerConfig.sampleCount                                      = 1;
		tracerConfig.maxDepth                                         = gvar_bounce_count.val.v_uint;
		tracerConfig.force_ray_marched_distance_sampling              = gvar_force_rm_distance.val.v_uint;
		tracerConfig.force_ray_marched_transmittance_sampling_al      = gvar_force_rm_transmittance_al.val.v_uint;
		tracerConfig.force_ray_marched_transmittance_sampling_env_map = gvar_force_rm_transmittance_env_map.val.v_uint;

		CVSArgs cvsArgs{};
		cvsArgs.defaultUpdateMode                = static_cast<CVSUpdateMode>(gvar_pin_update_mode.val.v_uint);
		cvsArgs.updateRate                       = gvar_pin_update_rate.val.v_uint;
		cvsArgs.traceUpdateArgs.rayCount         = gvar_pin_trace_update_ray_count.val.v_uint;
		cvsArgs.traceUpdateArgs.writePinStepSize = gvar_pin_write_pin_step_size.val.v_float;
		cvsArgs.rayMarchingCoefficient           = gvar_pin_ray_march_step_size_coefficient.val.v_float;
		cvsArgs.pinGridExtent.positionGridSize   = gvar_pin_pos_grid_size.val.v_uint;
		cvsArgs.pinGridExtent.directionGridSize  = gvar_pin_dir_grid_size.val.v_uint;
		cvsArgs.pinArgs.bitMaskIterations        = gvar_pin_bit_mask_iterations.val.v_uint;
		cvsArgs.pinArgs.jitterPos                = gvar_jitter_pos.val.v_float;
		cvsArgs.pinArgs.jitterDir                = gvar_jitter_dir.val.v_float;
		cvsArgs.forceFullUpdate                  = guiCatChanged(GUI_CAT_SCENE_MEDIUM_DENSITY);

		TraceDebugArgs debugArgs{};
		if (gvar_enable_debuging.val.v_bool)
		{
			debugArgs.pixelPos        = lastClickPos;
			debugArgs.enableHistogram = true;
			debugArgs.enablePtPlot    = true;

			debugArgs.ptPlotOptions.writeTotalContribution = gvar_pt_plot_write_total_contribution.val.v_bool;
			debugArgs.ptPlotOptions.writeIndirectDir       = gvar_pt_plot_write_indirect_dir.val.v_bool;
			debugArgs.ptPlotOptions.writeIndirectT         = gvar_pt_plot_write_indirect_t.val.v_bool;
			debugArgs.ptPlotOptions.writeIndirectWeight    = gvar_pt_plot_write_indirect_weight.val.v_bool;
			debugArgs.ptPlotOptions.bounce                 = gvar_pt_plot_bounce.val.v_uint;

			if (viewChange || selectPixel)
			{
				debugArgs.resetHistogram    = true;
				debugArgs.maxHistCount      = maxHistogramCount;
				debugArgs.maxHistValueCount = maxHistValueCount;
			}
		}

		TraceArgs commonTraceArgs{};
		commonTraceArgs.resources       = traceResources;
		commonTraceArgs.sceneParams     = sceneParams;
		sceneParams.cameraCI.extent     = cResolutions[gvar_eval_resolution.val.v_uint];
		commonTraceArgs.config          = tracerConfig;
		commonTraceArgs.cvsArgs         = cvsArgs;
		commonTraceArgs.enableDebugging = gvar_enable_debuging.val.v_bool;
		commonTraceArgs.debugArgs       = debugArgs;

		TraceArgs newTraceArgsLeft = commonTraceArgs;
		newTraceArgsLeft.cvsArgs.pinArgs.type = static_cast<PinType>(gvar_pin_mode_left.val.v_uint);
		newTraceArgsLeft.cvsArgs.pinArgs.sampleMode = static_cast<PinSampleMode>(gvar_sample_mode_left.val.v_uint);

		TraceArgs newTraceArgsRight = commonTraceArgs;
		newTraceArgsRight.cvsArgs.pinArgs.type = static_cast<PinType>(gvar_pin_mode_right.val.v_uint);
		newTraceArgsRight.cvsArgs.pinArgs.sampleMode = static_cast<PinSampleMode>(gvar_sample_mode_right.val.v_uint);

		if (firstFrame || leftClickInView || shaderRecompile)
		{
			traceArgsLeft.update(newTraceArgsLeft);
			traceArgsRight.update(newTraceArgsRight);
		}
		else
		{
			traceArgsLeft.updateFast(newTraceArgsLeft);
			traceArgsRight.updateFast(newTraceArgsRight);
		}
		IECToneMappingArgs toneMappingArgs{};
		toneMappingArgs.useTonemapping = gvar_tone_mapping_enable.val.v_bool;
		toneMappingArgs.whitePoint     = gvar_tone_mapping_whitepoint.val.v_float;
		toneMappingArgs.exposure       = gvar_tone_mapping_exposure.val.v_float;

		if (gvar_evaluate.val.v_bool)
		{
			OfflineRenderTask task{};
			task.name              = std::string(gvar_eval_name.val.v_char_array.data());
			task.argsRef           = tracerConfig;
			task.sceneParams       = sceneParams;
			task.cvsArgsLeft       = traceArgsLeft.cvsArgs;
			task.cvsArgsRight      = traceArgsRight.cvsArgs;
			task.config            = tracerConfig;
			task.config.sampleCount = gvar_sample_count.val.v_uint;
			task.resolution        = cResolutions[gvar_eval_resolution.val.v_uint];
			task.renderTimeRef     = gvar_eval_ref_target_time.val.v_float * 1000.0f; // sec to ms
			task.renderTimeCompare = gvar_eval_comp_target_time.val.v_float * 1000.0f; // sec to ms
			task.invocationsCompare = gvar_eval_comp_target_invokations.val.v_uint;
			task.mode              = static_cast<OfflineRenderMode>(gvar_eval_mode.val.v_uint);
			task.toneMappingArgs    = {};
			offlineRenderer.addTask(task);
		}
		gvar_remaning_tasks.val.v_uint = offlineRenderer.getTaskQueueSize();
		gvar_task_progress.val.v_float = offlineRenderer.getTaskProgress();
		offlineRenderer.cmdRunTick(cmdBuf);

		if ((gvar_eval_keep_realtime_render.val.v_bool || offlineRenderer.getTaskQueueSize() == 0) && !offlineMode)
		{
			iec.cmdRunEqualTime<TraceArgs>(cmdBuf, cmdTrace, traceArgsLeft, traceArgsRight, &gvar_timing_left.val.v_float, &gvar_timing_right.val.v_float);
			gvar_mse.val.v_float = iec.getMSE();
		}
		//// Show results
		Image swapchainImg = getSwapchainImage();

		if (!offlineMode)
		{
			getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);
			iec.showSplitView(cmdBuf, swapchainImg, splitViewCoef, getScissorRect(viewDimensions), toneMappingArgs);
			gState.uploadQueue->processUploadTasks(cmdBuf);
			cmdRenderGui(cmdBuf, swapchainImg);
		}
		else
		{
			VkExtent2D targetExtent = gState.io.extent;
			targetExtent.width      = targetExtent.width * offlineRenderer.getTaskProgress();
			getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.9, 0.1, 0.1, 1.0)).exec(cmdBuf);
			if (targetExtent.width > 0)
			{
				getCmdDrawRect(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.1, 0.9, 0.1, 1.0), targetExtent).exec(cmdBuf);
			}
			ImGui::EndFrame();
		}

		swapBuffers({cmdBuf});
		frameCount++;
	}
	//// Save settings
	vkDeviceWaitIdle(gState.device.logical);
	traceResourceCache.clearAll();
	mediumInstResCache.clearAll();
	offlineRenderer.destroy();
	GVar::storeAll(configPath + "last_session.json");
	gState.destroy();
}
#include "config.h"
#include "shader_interface.h"
#include "ui.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders";
using namespace glm;

GVar gvar_perlin_frequency{"Perlin frequency", 4.f, GVAR_FLOAT_RANGE, GUI_CAT_NOISE, {1.f, 10.f}};

GVar gvar_pt_plot_write_total_contribution{"Write total contribution", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_dir{"Write indirect direction", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_t{"Write indirect t", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_weight{"Write indirect weight", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_bounce{"Select Bounce", 0, GVAR_UINT_RANGE, GUI_CAT_PT_PLOT, {0, 5}};
GVar gvar_mse{"MSE: %.8f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_left{"Timing Left: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_right{"Timing Left: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};

int main()
{
	//// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();
	//// Image Estimator Comparator
	ImageEstimatorComparator iec = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, viewDimensions.width, viewDimensions.height);
	//// Init other stuff
	FixedCamera                                           cam          = FixedCamera(DefaultFixedCameraState());
	USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance>();
	USceneData scene;
	//// Load stuff:
	CmdBuffer cmdBuf = createCmdBuffer(gState.frame->stack);
	//// Load Geometry
	sceneBuilder.loadEnvMap("/envmap/2k/autumn_field_2k.hdr", glm::uvec2(64, 64));
#ifdef RAY_TRACING_SUPPORT
	GLSLInstance instance{};
	instance.cullMask = 0xFF;
	instance.mat      = getMatrix(vec3(0, 0.2, -0.3), vec3(0.0, 180.0, 0.0), 0.1);
	sceneBuilder.addModel(cmdBuf, "cornell_box/cornell_box.obj", &instance, 1);
	scene = sceneBuilder.create(cmdBuf, gState.heap);
	scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.heap));
#endif
	//// Load Medium
	PerlinNoiseArgs perlinArgs{};
	perlinArgs.scale         = 1000.0;
	perlinArgs.min           = 0.0;
	perlinArgs.max           = 100.0;
	perlinArgs.frequency          = gvar_perlin_frequency.val.v_float;
	perlinArgs.falloffAtEdge = true;
	const uint32_t mediumExtent1D = 64;
	VkExtent3D     mediumExtent{mediumExtent1D, mediumExtent1D, mediumExtent1D};
	Image          medium = createImage(gState.heap, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, mediumExtent);
	cmdTransitionLayout(cmdBuf, medium, VK_IMAGE_LAYOUT_GENERAL);
	GLSLMediumInstance mediumInstance{};
	mediumInstance.mat = getMatrix(vec3(-0.2, -0.2, -0.2), vec3(0, 0, 0), 0.4);
	mediumInstance.invMat = glm::inverse(mediumInstance.mat);
	mediumInstance.albedo  = vec3(1.0, 1.0, 1.0);
	Buffer mediumInstanceBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	cmdWriteCopy(cmdBuf, mediumInstanceBuffer, &mediumInstance, sizeof(GLSLMediumInstance));
	executeImmediat(cmdBuf);
	gState.updateSwapchainAttachments();
	// Main Loop
	uint32_t frameCount = 0;
	vec2 lastClickPos = vec2(1.0);
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
		//// Process Updates
		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		// Reset accumulation
		if (viewHasChanged || anySettingsChanged || shaderRecompiled || firstFrame)
		{
			iec.cmdReset(cmdBuf);
		}
		bool resetPlots = viewHasChanged || anySettingsChanged || shaderRecompiled || leftClickInView || firstFrame;
		// Regenerate noise
		if (settingsChanged[GUI_CAT_NOISE] || shaderRecompiled || firstFrame)
		{
			perlinArgs.frequency = gvar_perlin_frequency.val.v_float;
			getCmdPerlinNoise(medium, perlinArgs).exec(cmdBuf);
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
		traceArgs.maxDepth                  = 5;
		traceArgs.rayMarchStepSize          = 0.1;
		traceArgs.cameraCI                  = camCI;
		traceArgs.sceneData                 = scene;
		traceArgs.mediumInstanceBuffer      = mediumInstanceBuffer;
		traceArgs.mediumTexture             = medium;
		traceArgs.enableDebugging           = true;
		traceArgs.debugArgs.pixelPos        = lastClickPos;
		traceArgs.debugArgs.enableHistogram = true;
		traceArgs.debugArgs.enablePtPlot    = true;

		traceArgs.debugArgs.ptPlotOptions.writeTotalContribution = gvar_pt_plot_write_total_contribution.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.writeIndirectDir       = gvar_pt_plot_write_indirect_dir.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.writeIndirectT         = gvar_pt_plot_write_indirect_t.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.writeIndirectWeight    = gvar_pt_plot_write_indirect_weight.val.v_bool;
		traceArgs.debugArgs.ptPlotOptions.bounce                 = gvar_pt_plot_bounce.val.v_uint;

		if (leftClickInView)
		{
			traceArgs.debugArgs.enablePlot        = true;
			traceArgs.debugArgs.maxPlotCount      = 10;
			traceArgs.debugArgs.maxPlotValueCount = 1000;
		}

		if (resetPlots)
		{
			traceArgs.debugArgs.resetHistogram = true;
			traceArgs.debugArgs.maxHistCount   = 10;
			traceArgs.debugArgs.maxHistValueCount = 1000000;
		}
		iec.cmdRun<TraceArgs>(cmdBuf, cmdTrace, traceArgs, traceArgs, &gvar_timing_left.val.v_float, &gvar_timing_right.val.v_float);
		//// Show results
		gvar_mse.val.v_float = iec.getMSE();
		Image swapchainImg = getSwapchainImage();
		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);
		iec.showSplitView(cmdBuf, swapchainImg, 0.5, getScissorRect(viewDimensions));
		cmdRenderGui(cmdBuf, swapchainImg);

		swapBuffers({cmdBuf});
		frameCount++;
	}
	gState.destroy();
}
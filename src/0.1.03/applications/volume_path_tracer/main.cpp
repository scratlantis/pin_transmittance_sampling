#include "config.h"
#include "utility.h"
#include "shader_interface.h"
#include "medium/Medium.h"
#include "medium/medium_strategies.h"
#include "visualization/pin_visualization.h"
#include "path_tracing/ComparativePathTracer.h"
#include "path_tracing/pt_strategies.h"

// State and output dir must be defined in the main file
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

// Gui variable

GVar gvar_model = {"Model", 0, GVAR_ENUM, GENERAL, {"Cornell Box"}};
GVar gvar_image_resolution{"Image Resolution", 64, GVAR_UINT_RANGE, PERLIN_NOISE_SETTINGS, {16, 128}};
GVar gvar_mse{"MSE : %.8f E-3", 0.0f, GVAR_DISPLAY_VALUE, METRICS };



std::vector<GVar *> gVars =
{
        // clang-format off
        &gvar_model,
		&gvar_perlin_frequency0,
		&gvar_perlin_frequency1,
		&gvar_perlin_scale0,
		&gvar_perlin_scale1,
		&gvar_perlin_falloff,
		&gvar_medium_albedo_r,
		&gvar_medium_albedo_g,
		&gvar_medium_albedo_b,
		&gvar_image_resolution,
		&gvar_pin_count,
		&gvar_pin_transmittance_value_count,
		&gvar_pin_count_per_grid_cell,
		&gvar_pin_grid_size,
		&gvar_cursor_pos_x,
		&gvar_cursor_pos_y,
		&gvar_cursor_pos_z,
		&gvar_cursor_dir_phi,
		&gvar_cursor_dir_theta,
		&gvar_mse,
        // clang-format on
};

ShaderConst sConst{};

// Models
ModelInfo cursor = {"arrow_cursor/arrow_cursor.obj", vec3(0.0, 0.0, 0.0), 0.05, vec3(0.0, 0.0, 0.0)};
ModelInfo cornellBox = {"cornell_box/cornell_box.obj", vec3(0,0.2,-0.3), 0.1, vec3(0.0,180.0,0.0)};
std::vector<ModelInfo>    models               = {cornellBox};
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

const uint32_t viewTypeCount = 2;

int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();

	// Application Specific Parameters
	Params params = DefaultParams();

	// May be added to the global state in the future
	HdrImagePdfCache pdfCache = HdrImagePdfCache(gState.heap);
	USceneBuilder<GLSLVertex, GLSLMaterial> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial>(&pdfCache);

	// Medium
	Medium          medium          = Medium();
	Buffer      mediumInstanceBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	PinStateManager pinStateManager = PinStateManager(&medium);

	// Scene
	FixedCamera cam = FixedCamera(DefaultFixedCameraCI());
	uint32_t modelIndexLastFrame = 0;
	uint32_t viewType = 0;
	USceneData scene;
	ModelInfo  model;

	// Path Tracer
	ComparativePathTracer pathTracer = ComparativePathTracer(0.8, 1.0);


	Buffer mseBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(float));




	// Persistent Resources:
	// HDR Images for path tracing
	Image img_pt = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	Image img_debug = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL);
	Image img_pt_accumulation = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	gState.updateSwapchainAttachments();
	// Uniform Buffers
	sConst.alloc();

	// Load static models
	CmdBuffer cmdBuf = createCmdBuffer(gState.heap);
	ModelData cursorModel = gState.modelCache->fetch<GLSLVertex>(cmdBuf, cursor.path, 0);
	executeImmediat(cmdBuf);

	bool debugView = false;

	float splitCoef = 0.5;

	// Main Loop
	for (uint cnt = 0; !gState.io.shouldTerminate(); cnt++)
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		if (gState.io.keyPressedEvent[GLFW_KEY_T]) // Reset camera
		{
			cam = FixedCamera(DefaultFixedCameraCI());
		}
		if (modelIndexLastFrame != gvar_model.val.v_uint || cnt == 0) // Load Scene
		{
			if (cnt != 0)
			{
				scene.garbageCollect();
			}

			modelIndexLastFrame = gvar_model.val.v_uint;
			model = models[modelIndexLastFrame];
			CmdBuffer cmdBuf = createCmdBuffer(gState.heap);

			// Solid Geometry
			sceneBuilder.reset();
			sceneBuilder.addModel(cmdBuf, model.path, model.getObjToWorldMatrix());
			scene = sceneBuilder.create(cmdBuf, gState.heap, SCENE_LOAD_FLAG_ALLOW_RASTERIZATION);
			scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.heap));
			executeImmediat(cmdBuf);
		}
		bool viewHasChanged = cam.keyControl(0.016);
		if (gState.io.mouse.rightPressed)
		{
			viewHasChanged = viewHasChanged || cam.mouseControl(0.016);
		}
		if (gState.io.mouse.middleEvent && gState.io.mouse.middlePressed)
		{
			debugView = !debugView;
		};
		if (gState.io.keyPressedEvent[GLFW_KEY_E])
		{
			viewType = (viewType + 1) % viewTypeCount;
		}
		if (gState.io.keyPressedEvent[GLFW_KEY_Q])
		{
			viewType = (viewTypeCount + viewType - 1) % viewTypeCount;
		}
		
		CmdBuffer  cmdBuf       = createCmdBuffer(gState.frame->stack);
		Image      swapchainImg = getSwapchainImage();


		// Update medium
		GLSLMediumInstance mediumInstance{};
		mediumInstance.mat    = params.initialMediumMatrix;
		mediumInstance.invMat = glm::inverse(mediumInstance.mat);
		mediumInstance.albedo = vec3(gvar_medium_albedo_r.val.v_float, gvar_medium_albedo_g.val.v_float, gvar_medium_albedo_b.val.v_float);
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
		if ( cnt <= 1 || viewHasChanged || gState.io.mouse.leftPressed && gState.io.mouse.leftPressed && gState.io.mouse.pos.x < 0.2 * gState.io.extent.width)
		{
			pathTracer.reset(cmdBuf, &referencePathTracer, &pinPathTracer);
		}

		if (gState.io.mouse.leftPressed && gState.io.mouse.pos.x > 0.2 * gState.io.extent.width)
		{
			splitCoef = glm::clamp<float>(splitCoef + gState.io.mouse.change.x / (0.8f * gState.io.extent.width), 0.0, 1.0);
		}


		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.2, 0.2, 0.2, 1.0)).exec(cmdBuf);


		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		// Path tracing
		if (!debugView)
		{
			RenderInfo renderInfo           = RenderInfo();
			renderInfo.frameIdx             = cnt;
			renderInfo.pCamera              = &cam;
			renderInfo.pMediun              = &medium;
			renderInfo.mediumInstanceBuffer = mediumInstanceBuffer;
			renderInfo.pSceneData           = &scene;
			pathTracer.render(cmdBuf, renderInfo);

			switch (viewType)
			{
				case VIEW_TYPE_SPLIT:
					pathTracer.showSplitView(cmdBuf, swapchainImg, splitCoef, getScissorRect(0.2f, 0.f, 0.8f, 1.0f));
					break;
				case VIEW_TYPE_DIFF:
					pathTracer.showDiff(cmdBuf, swapchainImg, getScissorRect(0.2f, 0.f, 0.8f, 1.0f));
					break;
				default:
					break;
			}

			// Compute MSE
			gvar_mse.val.v_float = 1000.0*pathTracer.computeMSE(cmdBuf);

			//pathTracer.showDiff(cmdBuf, img_debug, getScissorRect(0.0f, 0.f, 1.0f, 1.0f));
			//getCmdAdvancedCopy(img_debug, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			//                    VkRect2D_OP(img_debug->getExtent2D()), getScissorRect(0.2f, 0.f, 0.8f, 1.0f)).exec(cmdBuf);
			//gvar_mse.val.v_float = pathTracer.computeMSE2(cmdBuf, img_debug);

			
		}
		// Rasterization for debugging
		else
		{
			img_pt->setClearValue(ClearValue(0.0f, 0.0f, 0.0f, 1.0f));
			cmdShowTriangles<GLSLVertex>(cmdBuf, gState.frame->stack, img_pt, scene.vertexBuffer, scene.indexBuffer, &cam, model.getObjToWorldMatrix(), true);
			cmdShowBoxFrame(cmdBuf, gState.frame->stack, img_pt, &cam, mediumInstance.mat, false, vec4(0.0, 0.0, 1.0, 1.0));


			glm::mat4 cursorMatrix = glm::translate(glm::mat4(1.0), vec3(gvar_cursor_pos_x.val.v_float, gvar_cursor_pos_y.val.v_float, gvar_cursor_pos_z.val.v_float));
			cursorMatrix = glm::rotate(cursorMatrix, gvar_cursor_dir_theta.val.v_float, vec3(0.0, 0.0, 1.0));
			cursorMatrix = glm::rotate(cursorMatrix, gvar_cursor_dir_phi.val.v_float, vec3(0.0, 1.0, 0.0));
			cursorMatrix           = mediumInstance.mat * cursorMatrix * cursor.getObjToWorldMatrix();
			cmdShowAlbedo(cmdBuf, gState.frame->stack, img_pt, cursorModel, &cam, cursorMatrix, false);
			cmdVisualizePins(cmdBuf, gState.frame->stack, img_pt, medium.pins, pinStateManager.pinState, &cam, mediumInstance.mat, false, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			getCmdAdvancedCopy(img_pt, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			                VkRect2D_OP(img_pt->getExtent2D()), getScissorRect(0.2f, 0.f, 0.8f, 1.0f)).exec(cmdBuf);
		}
		// Add gui
		{
			gvar_gui::buildGui(gVars, {"General", "Noise Function", "Pins", "Visualization", "Metrics"}, getScissorRect(0.f, 0.f, 0.2, 1.0));
			shader_console_gui::buildGui(getScissorRect(0.2f, 0.f, 0.8f, 1.0f));
			cmdRenderGui(cmdBuf, swapchainImg);
		}

		swapBuffers({cmdBuf});
	}
	gState.destroy();
}
#include "config.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

// Gui variable
static GVar                gvar_color = {"color", {1.0, 0.0, 0.0}, GVAR_VEC3, 0};
static std::vector<GVar *> gVars =
    {
        &gvar_color
};

static ShaderConst sConst{};

int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();

	// May be added to the global state in the future
	HdrImagePdfCache pdfCache = HdrImagePdfCache(gState.heap);

	USceneBuilder<GLSLVertex, GLSLMaterial> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial>(&pdfCache);

	// Camera
	FixedCamera cam = FixedCamera(FixedCameraCI_Default());

	// Persistent Resources:
	// HDR Image for path tracing
	Image img_pt = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	// Uniform Buffers
	sConst.alloc();


	gState.updateSwapchainAttachments();

	CmdBuffer cmdBuf = createCmdBuffer(gState.heap);
	sceneBuilder.reset();
	sceneBuilder.addModel(cmdBuf, "cornell_box/cornell_box.obj", glm::scale(glm::mat4(1.0),vec3(0.1)));
	USceneData scene = sceneBuilder.create(cmdBuf, gState.heap, SCENE_LOAD_FLAG_ALLOW_RASTERIZATION);
	/*scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.heap));*/
	executeImmediat(cmdBuf);

	// Main Loop
	for (uint cnt = 0; !gState.io.shouldTerminate(); cnt++)
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		cam.keyControl(0.016);
		if (gState.io.mouse.rightPressed)
		{
			cam.mouseControl(0.016);
		}
		CmdBuffer  cmdBuf       = createCmdBuffer(gState.frame->stack);
		Image      swapchainImg = getSwapchainImage();
		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.2, 0.2, 0.2, 1.0)).exec(cmdBuf);
		// Path tracing
		if (1)
		{
			scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.frame->stack));
			// Config general parameters
			ComputeCmd computeCmd = ComputeCmd(img_pt->getExtent2D(), shaderPath + "path_tracing/pt.comp", {{"FORMAT1", getGLSLFormat(img_pt->getFormat())}});
			sConst.write(cmdBuf, computeCmd, img_pt->getExtent2D(), cam, cnt, gVars);

			// Bind Constants
			bind_block_3(computeCmd, sConst);

			// Bind Target
			computeCmd.pushDescriptor(img_pt, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			// Bind Scene
			bind_block_9(computeCmd, scene);

			computeCmd.exec(cmdBuf);

		}
		else
		{
			img_pt->setClearValue(ClearValue(0.0f, 0.0f, 0.0f, 1.0f));
			cmdShowTriangles<GLSLVertex>(cmdBuf, gState.frame->stack, img_pt, scene.vertexBuffer, scene.indexBuffer, &cam, 0.1);
		}
		// Composition
		{
			DrawCmd drawCmd = getCmdAdvancedCopy(img_pt, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			                                     VkRect2D_OP(img_pt->getExtent2D()), getScissorRect(0.2f, 0.f, 0.8f, 1.0f));
			drawCmd.exec(cmdBuf);
			gvar_gui::buildGui(gVars, {"Catergory 1"}, getScissorRect(0.f,0.f,0.2,1.0));
			shader_console_gui::buildGui(getScissorRect(0.2f, 0.f, 0.8f, 1.0f));
			cmdRenderGui(cmdBuf, swapchainImg);
		}
		swapBuffers({cmdBuf});
	}
	gState.destroy();
}
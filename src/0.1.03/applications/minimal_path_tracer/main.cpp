#include "config.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

// Gui variable
static GVar                gvar_color = {"color", {1.0, 0.0, 0.0}, GVAR_VEC3, 0};
static std::vector<GVar *> gVars =
    {
        &gvar_color
};

int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();

	// Camera
	FixedCamera cam = FixedCamera(FixedCameraCI_Default());

	// Persistent Resources:
	// HDR Image for path tracing
	Image  img_pt = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	// Uniform Buffers
	Buffer ubo_frame = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLFrame));
	Buffer ubo_view  = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLFrame));
	Buffer ubo_params = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLParams));
	

	gState.updateSwapchainAttachments();
	// Main Loop
	for (uint cnt = 0; !gState.io.shouldTerminate(); cnt++)
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		CmdBuffer  cmdBuf       = createCmdBuffer(gState.frame->stack);
		Image      swapchainImg = getSwapchainImage();
		GLSLParams params       = guiParams(gVars);
		cmdWriteCopy(cmdBuf, ubo_params, &params, sizeof(GLSLParams));

		// Path tracing
		{
			ComputeCmd computeCmd = ComputeCmd(img_pt->getExtent2D(), shaderPath + "path_tracing/pt.comp");
			configShaderPrelude1(cmdBuf, computeCmd, img_pt->getExtent2D(), cam, cnt, ubo_frame, ubo_view, ubo_params);
			computeCmd.pushDescriptor(img_pt, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			computeCmd.exec(cmdBuf);
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
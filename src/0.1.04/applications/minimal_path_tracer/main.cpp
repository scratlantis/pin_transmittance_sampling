#include "config.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
using namespace glm;

int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);

	FixedCamera                                           cam                 = FixedCamera(DefaultFixedCameraState());
	Image img_pt              = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	Image img_pt_accumulation = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	gState.updateSwapchainAttachments();

	USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance>();
	CmdBuffer cmdBuf = createCmdBuffer(gState.frame->stack);
	//// Load Scene
	cmdFill(cmdBuf, img_pt_accumulation, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0, 0.0, 1.0, 1.0));
	executeImmediat(cmdBuf);

	// Main Loop
	while (!gState.io.shouldTerminate())
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		bool viewHasChanged = cam.keyControl(0.016);
		if (gState.io.mouse.rightPressed)
		{
			viewHasChanged = viewHasChanged || cam.mouseControl(0.016);
		}

		viewHasChanged = viewHasChanged || gState.io.swapchainRecreated();

		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		Image     swapchainImg = getSwapchainImage();
		if (viewHasChanged)
		{
			cmdFill(cmdBuf, img_pt_accumulation, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0,0.0,1.0,1.0));
		}
		// Path tracing
		{

		}
		// Accumulation
		{
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			//getCmdAccumulate(img_pt, img_pt_accumulation, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
			getCmdNormalize(img_pt_accumulation, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			                VkRect2D_OP(img_pt_accumulation->getExtent2D()), getScissorRect()).exec(cmdBuf);
		}

		swapBuffers({cmdBuf});
	}

	gState.destroy();
}
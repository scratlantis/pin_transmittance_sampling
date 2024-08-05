#include "config.h"
#include <vka/vka.h>
using namespace vka;
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

//Gui variable
GVar                gvar_color = {"color", {1.0, 0.0, 0.0}, GVAR_VEC3, 0};
std::vector<GVar *> gVars =
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

	// HDR Image for path tracing
	Image ptTargetImg = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);




	gState.updateSwapchainAttachments();
	// Main Loop
	while (!gState.io.shouldTerminate())
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		Image     swapchainImg = getSwapchainImage();
		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		// Path tracing

		// Composition
		DrawCmd drawCmd = getCmdAdvancedCopy(ptTargetImg, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		drawCmd.exec(cmdBuf);
		gvar_gui::buildGui(gVars, {"Catergory 1"}, getScissorRect(0.f,0.f,0.2,1.0));
		shader_console_gui::buildGui(getScissorRect(0.2f, 0.f, 0.8f, 1.0f));
		cmdRenderGui(cmdBuf, swapchainImg);
		swapBuffers({cmdBuf});
	}
	gState.destroy();
}
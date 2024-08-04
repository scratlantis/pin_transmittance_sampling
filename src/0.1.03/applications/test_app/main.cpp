#include "config.h"
#include <vka/vka.h>
using namespace vka;
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = AdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();

	// Main Loop
	while (!gState.io.shouldTerminate())
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		Image     swapchainImg = getSwapchainImage();
		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		DrawCmd   drawCmd      = getCmdFill(swapchainImg, {1.0, 0.0, 0.0, 1.0});
		drawCmd.exec(cmdBuf);
		swapBuffers({cmdBuf});
	}
	gState.destroy();
}
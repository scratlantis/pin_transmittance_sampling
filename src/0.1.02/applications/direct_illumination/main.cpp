#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <vka/vka.h>
#include <vka/state_objects/GlfwWindow.h>
#include <vka/render/common.h>
#include <vka/geometry/common.h>
#include "config.h"

using namespace vka;
AppState gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gResourceBaseDir = RESOURCE_BASE_DIR;
std::vector<GVar *> gVars = {};
int main()
{
	// Global State Initialization
	DeviceCI      deviceCI = D3VKPTDeviceCI(APP_NAME);
	IOControlerCI ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow    window   = GlfwWindow();
	gState.init(deviceCI, ioCI, &window);
	FixedCamera camera = FixedCamera(FixedCameraCI_Default());
	GvarGui gui = GvarGui();

	// Init:
	// SwapchainImage
	// Framebuffer Cache

	// Upload:

	// Main loop:
	while (!gState.io.shouldTerminate())
	{
		if (gState.io.keyEvent[GLFW_KEY_R] && gState.io.keyPressed[GLFW_KEY_R])
		{
			vkDeviceWaitIdle(gState.device.logical);
			gState.cache->clearShaders();
		}

		// StartCmdBuf
		// Render
		// SwapBuffers
	}
	// Cleanup
	vkDeviceWaitIdle(gState.device.logical);
	gState.destroy();
}
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "stdio.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <framework/vka/global_state.h>
#include <framework/vka/initializers/misc.h>
#include <framework/vka/context/GlfwWindow.h>

using namespace vka;

vka::AppState gState;

int main()
{
	DeviceCI deviceCI = D3VKPTDeviceCI("test_application");
	IOControlerCI ioCI     = DefaultIOControlerCI("test_window", 400,400);
	Window* window = new vka::GlfwWindow();

	gState.init(deviceCI, ioCI, window);
	
	while (!gState.io.shouldTerminate())
	{
		gState.nextFrame();
	}
	gState.destroy();
	delete window;
}
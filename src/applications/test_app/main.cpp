#include "stdio.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <framework/vka/core/container/StructureChain.h>
#include <framework/vka/global_state.h>

//using namespace vka;

//Device device;
//Swapchain mainSwapchaine;

int main()
{
	glm::vec2 x;
	//DeviceCreateInfo deviceCI = DefaultDeviceCreateInfo();
	//std::vector<SwapchainCreateInfo> swapchainCIs = {DefaultSwapchainCreateInfo("test_application", 500, 500)};
	//std::vector<Swapchain> swapchaines;
	//initVulkanGLFW(deviceCI, swapchainCIs, device, swapchaines);
	//mainSwapchaine            = swapchaines[0];
	//
	//while (!mainSwapchaine.window->shouldClose())
	//{
	//	mainSwapchaine.readInputs();
	//}
	//
	//
	//swapchaines[0] = mainSwapchaine;
	//shutdownVulkanGLFW(device, swapchaines);
	printf("Hello World\n");
}
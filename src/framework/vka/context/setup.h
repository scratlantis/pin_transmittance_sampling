#pragma once
#pragma once
#include "../global_state.h"
#include "../core/macros/macros.h"
#include "../core/container/StructureChain.h"

namespace vka
{

struct QueueFamilyIndices
{
	int graphicsFamily     = -1;
	int presentationFamily = -1;
	int computeFamily      = -1;

	int graphicsFamilyCount     = 0;
	int presentationFamilyCount = 0;
	int computeFamilyCount      = 0;

	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0 && computeFamily >= 0;
	}
};




static QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

static SwapChainDetails getSwapchainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

static bool checkInstanceExtensionSupport(std::vector<const char *> *checkExtensions);

static bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char *> deviceExtensions);

static int checkDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char *> deviceExtensions);

static VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags, VkPhysicalDevice physicalDevice);

/*
static void initVulkanGLFW(ApiContextCreateInfo contextCI, std::vector<SwapchainCreateInfo> swapchainCI, ApiContext &context, std::vector<Swapchain> &swapchain);

static void shutdownVulkanGLFW(ApiContext &context, std::vector<Swapchain> &swapchain);
*/
}        // namespace vka
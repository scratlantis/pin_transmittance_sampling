#pragma once
#include <vka/core/types/setup.h>

namespace vka
{

SwapChainDetails   getSwapchainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
void               selectQueues(int universalQueueCount, int computeQueueCount, int &universalQueueFamily, int &computeQueueFamily);
bool               checkInstanceExtensionSupport(std::vector<const char *> *checkExtensions);
bool               checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char *> deviceExtensions);
int                checkDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char *> deviceExtensions);
VkFormat           chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags, VkPhysicalDevice physicalDevice);
}        // namespace vka
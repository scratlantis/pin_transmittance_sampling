#pragma once

// #include <framework/vka/helper/misc.h>
// #include <framework/vka/core/core_common.h>
// #include <framework/vka/core/core_config.h>
// #include <framework/vka/core/core_debug.h>
// #include <iostream>
// #include <framework/vka/helper/math.h>
// #include <framework/vka/container/StructureChain.h>
// #include <framework/vka/helper/vk_misc.h>
// #include <framework/vka/helper/vk_initializers.h>
// #include <framework/vka/window/GlfwWindow.h>
// #include <framework/vka/objects/obj_application_context.h>

// #include <framework/vka/helper/misc.h>
// #include <vector>
/*
namespace vka
{
struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR        surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentationMode;
};

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

static QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices{};
	uint32_t           queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyList.data());
	int index = 0;
	for (const auto &queueFamily : queueFamilyList)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = index;
			indices.graphicsFamilyCount += 1;
		}
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			indices.computeFamily = index;
			indices.computeFamilyCount += 1;
		}
		VkBool32 presentation_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentation_support);
		if (queueFamily.queueCount > 0 && presentation_support)
		{
			indices.presentationFamily = index;
			indices.presentationFamilyCount += 1;
		}
		if (indices.isValid())
		{
			break;
		}
		index++;
	}
	return indices;
}

static SwapChainDetails getSwapchainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	SwapChainDetails swapchainDetails{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainDetails.surfaceCapabilities);
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	swapchainDetails.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapchainDetails.formats.data());
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentationCount, nullptr);
	if (presentationCount > 0)
	{
		swapchainDetails.presentationMode.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentationCount, swapchainDetails.presentationMode.data());
	}
	return swapchainDetails;
}

static bool checkInstanceExtensionSupport(std::vector<const char *> *checkExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	for (const auto &checkExtension : *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto &extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}
	return true;
}

static bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char *> deviceExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	if (extensionCount == 0)
	{
		return false;
	}
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

	for (const auto &deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto &extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}
	return true;
}

static int checkDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char *> deviceExtensions)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	QueueFamilyIndices indices             = getQueueFamilies(physicalDevice, surface);
	bool               extensionsSupported = checkDeviceExtensionSupport(physicalDevice, deviceExtensions);
	bool               swapChainValid      = false;
	if (extensionsSupported)
	{
		SwapChainDetails swapChainDetails = getSwapchainDetails(physicalDevice, surface);
		swapChainValid                    = !swapChainDetails.presentationMode.empty() && !swapChainDetails.formats.empty();
	}
	if (!(indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy))
	{
		return 0;
	}
	else if (deviceProperties.vendorID == INTEL_VENDOR_ID)
	{
		return 1;
	}
	else if (deviceProperties.vendorID == AMD_VENDOR_ID)
	{
		return 2;
	}
	else if (deviceProperties.vendorID == NVIDIA_VENDOR_ID)
	{
		return 3;
	}
	return 1;
}

static VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags, VkPhysicalDevice physicalDevice)
{
	for (VkFormat format : formats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}
	throw std::runtime_error("Failed to find supported format!");
}

static void createInstance(const ApiContextCreateInfo &contextCI, VkInstance &instance)
{
	VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
	appInfo.pApplicationName = contextCI.applicationName.c_str();
	appInfo.pEngineName      = contextCI.applicationName.c_str();
	appInfo.apiVersion       = API_VERSION;

	std::vector<const char *> instanceExtensions;
	if (contextCI.enabledInstanceExtensions.size() > 0)
	{
		for (const char *enabledExtension : contextCI.enabledInstanceExtensions)
		{
			if (std::find(contextCI.enabledInstanceExtensions.begin(), contextCI.enabledInstanceExtensions.end(), enabledExtension) == contextCI.enabledInstanceExtensions.end())
			{
				std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
			}
			instanceExtensions.push_back(enabledExtension);
		}
	}

	VkInstanceCreateInfo instanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (!instanceExtensions.empty())
	{
		IF_VALIDATION(
		    instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);)
		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	const char *validationLayerName        = "VK_LAYER_KHRONOS_validation";
	instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
	instanceCreateInfo.enabledLayerCount   = 1;
	ASSERT_VULKAN(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
	IF_VALIDATION(
	    uint32_t instanceLayerCount;
	    vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
	    vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
	    bool validationLayerPresent = false;
	    for (VkLayerProperties layer
	         : instanceLayerProperties) {
		    if (strcmp(layer.layerName, validationLayerName) == 0)
		    {
			    validationLayerPresent = true;
			    break;
		    }
	    } if (validationLayerPresent) {
		    instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
		    instanceCreateInfo.enabledLayerCount   = 1;
	    } else {
		    std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
	    })
}

static void selectPhysicalDevice(const ApiContextCreateInfo &contextCI, VkInstance &instance, VkSurfaceKHR &surface, VkPhysicalDevice &physicalDevice)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("Can not find GPU's that support Vulkan Instance!");
	}

	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	int maxDevicePriority = 0;
	for (const auto &device : deviceList)
	{
		int devicePriority = checkDeviceSuitable(device, surface, contextCI.enabledDeviceExtensions);
		if (devicePriority > maxDevicePriority)
		{
			physicalDevice    = device;
			maxDevicePriority = devicePriority;
		}
	}
	if (maxDevicePriority == 0)
	{
		throw std::runtime_error("Can not find suitable GPU!");
	}
}

static void createLogicalDevice(const ApiContextCreateInfo &contextCI, const VkPhysicalDevice &physicalDevice, const VkSurfaceKHR &surface, VkDevice &logicalDevice, std::vector<VkQueue> &queues)
{
	QueueFamilyIndices                   indices = getQueueFamilies(physicalDevice, surface);
	std::vector<VkDeviceQueueCreateInfo> queueCIs;
	VkDeviceQueueCreateInfo              queueCI{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
	std::vector<float>                   piority(contextCI.queueCount);
	for (size_t i = 0; i < contextCI.queueCount; i++)
	{
		piority[i] = 1.0f;
	}
	queueCI.queueFamilyIndex = indices.graphicsFamily;
	queueCI.pQueuePriorities = piority.data();
	queueCI.queueCount       = contextCI.queueCount;
	queueCIs.push_back(queueCI);

	VkDeviceCreateInfo logicalDeviceCI{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	logicalDeviceCI.queueCreateInfoCount    = VKA_COUNT(queueCIs);
	logicalDeviceCI.pQueueCreateInfos       = queueCIs.data();
	logicalDeviceCI.enabledExtensionCount   = VKA_COUNT(contextCI.enabledDeviceExtensions);
	logicalDeviceCI.ppEnabledExtensionNames = contextCI.enabledDeviceExtensions.data();
	logicalDeviceCI.pNext                   = contextCI.enabledFeatures.chainNodes();

	ASSERT_VULKAN(vkCreateDevice(physicalDevice, &logicalDeviceCI, nullptr, &logicalDevice));
	queues.resize(contextCI.queueCount);
	for (size_t i = 0; i < contextCI.queueCount; i++)
	{
		vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, i, &queues[i]);
	}
}

static void createSwapchain(const SwapchainCreateInfo &swapchainCI, Swapchain &swapchain, const ApiContext &context, VkSwapchainKHR oldSwapchain)
{
	if (!swapchain.hasWindow)
	{
		SwapChainDetails   swapChainDetails = getSwapchainDetails(context.physicalDevice, swapchain.surface);
		VkSurfaceFormatKHR surfaceFormat    = swapChainDetails.formats[0];        // Must exist after vulkan specifications
		selectByPreference(swapChainDetails.formats, swapchainCI.preferedFormats, surfaceFormat);
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		selectByPreference(swapChainDetails.presentationMode, swapchainCI.preferedPresentModes, presentMode);
		VkExtent2D extent = swapChainDetails.surfaceCapabilities.currentExtent;
		if (extent.width == std::numeric_limits<uint32_t>::max())
		{
			clamp(swapchain.window->size(), swapChainDetails.surfaceCapabilities.minImageExtent, swapChainDetails.surfaceCapabilities.maxImageExtent);
		}

		uint32_t imageCount = swapchainCI.preferedSwapchainImageCount;
		imageCount          = std::max(imageCount, swapChainDetails.surfaceCapabilities.minImageCount);
		if (swapChainDetails.surfaceCapabilities.maxImageCount > 0)        // 0 == limitless
		{
			imageCount = std::min(imageCount, swapChainDetails.surfaceCapabilities.maxImageCount);
		}

		VkSwapchainCreateInfoKHR vkSwapchainCI{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
		vkSwapchainCI.surface          = swapchain.surface;
		vkSwapchainCI.imageFormat      = surfaceFormat.format;
		vkSwapchainCI.imageColorSpace  = surfaceFormat.colorSpace;
		vkSwapchainCI.presentMode      = presentMode;
		vkSwapchainCI.imageExtent      = extent;
		vkSwapchainCI.minImageCount    = imageCount;
		vkSwapchainCI.imageArrayLayers = 1;
		vkSwapchainCI.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		vkSwapchainCI.preTransform     = swapChainDetails.surfaceCapabilities.currentTransform;
		vkSwapchainCI.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		vkSwapchainCI.clipped          = VK_TRUE;

		QueueFamilyIndices indices = getQueueFamilies(device.physical, swapchain.surface);

		if (indices.graphicsFamily != indices.presentationFamily)
		{
			uint32_t queueFamilyIndices[] = {
			    (uint32_t) indices.graphicsFamily,
			    (uint32_t) indices.presentationFamily};

			vkSwapchainCI.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			vkSwapchainCI.queueFamilyIndexCount = 2;
			vkSwapchainCI.pQueueFamilyIndices   = queueFamilyIndices;
		}
		else
		{
			vkSwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		vkSwapchainCI.oldSwapchain = oldSwapchain;
		ASSERT_VULKAN(vkCreateSwapchainKHR(device.logical, &vkSwapchainCI, nullptr, &swapchain.vkSwapchain));
		swapchain.format = surfaceFormat.format;
		swapchain.extent = extent;
		vkGetSwapchainImagesKHR(device.logical, swapchain.vkSwapchain, &swapchain.imageCount, nullptr);
		swapchain.images.resize(swapchain.imageCount);
		swapchain.imageViews.resize(swapchain.imageCount);
		vkGetSwapchainImagesKHR(device.logical, swapchain.vkSwapchain, &swapchain.imageCount, swapchain.images.data());
		swapchain.hasWindow = true;

		for (size_t i = 0; i < swapchain.imageCount; i++)
		{
			ImageViewCreateInfo<Swapchain> imageViewCI = ImageViewCreateInfo<Swapchain>(swapchain.images[i], swapchain.format);
			ASSERT_VULKAN(vkCreateImageView(device.logical, &imageViewCI, nullptr, &swapchain.imageViews[i]));
		}
	}

	if (!swapchain.hasSyncObjects && swapchainCI.createSyncObjects)
	{
		swapchain.inFlightFences.resize(swapchain.imageCount);
		swapchain.imageAvailableSemaphores.resize(swapchain.imageCount);
		swapchain.renderFinishedSemaphores.resize(swapchain.imageCount);

		VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
		VkFenceCreateInfo     fenceCreateInfo     = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		fenceCreateInfo.flags                     = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < swapchain.imageCount; i++)
		{
			ASSERT_VULKAN(vkCreateFence(device.logical, &fenceCreateInfo, NULL, &(swapchain.inFlightFences[i])));
			ASSERT_VULKAN(vkCreateSemaphore(device.logical, &semaphoreCreateInfo, NULL, &(swapchain.imageAvailableSemaphores[i])));
			ASSERT_VULKAN(vkCreateSemaphore(device.logical, &semaphoreCreateInfo, NULL, &(swapchain.renderFinishedSemaphores[i])));
		}
		swapchain.hasSyncObjects = true;
	}
}

static void initVulkanGLFW(DeviceCreateInfo deviceCI, std::vector<SwapchainCreateInfo> swapchainCI, Device &device, std::vector<Swapchain> &swapchain)
{
	swapchain.resize(swapchainCI.size());
	uint32_t     glfwExtensionsCount = 0;
	const char **glfwExtensions;
	glfwInit();
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	for (size_t i = 0; i < glfwExtensionsCount; i++)
	{
		deviceCI.enabledInstanceExtensions.push_back(glfwExtensions[i]);
	}
	createInstance(deviceCI, device.instance);
	for (size_t i = 0; i < swapchainCI.size(); i++)
	{
		swapchain[i].window  = std::shared_ptr<GlfwWindowWrapper>(new GlfwWindowWrapper(swapchainCI[i], device.instance));
		swapchain[i].surface = swapchain[i].window->getSurface();
	}
	// window 0 is main window
	selectPhysicalDevice(deviceCI, device.instance, swapchain[0].surface, device.physical);
	createLogicalDevice(deviceCI, device.physical, swapchain[0].surface, device.logical, device.queues);
	for (size_t i = 0; i < swapchainCI.size(); i++)
	{
		createSwapchain(swapchainCI[i], swapchain[i], device, VK_NULL_HANDLE);
	}
}

static void shutdownVulkanGLFW(Device &device, std::vector<Swapchain> &swapchain)
{
	for (size_t i = 0; i < swapchain.size(); i++)
	{
		swapchain[i].destroy(device);
	}

	device.destroy();

	glfwTerminate();
}

void Swapchain::destroyVkSwapchain(VkDevice &device)
{
	if (hasWindow)
	{
		for (size_t i = 0; i < imageViews.size(); i++)
		{
			vkDestroyImageView(device, imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, vkSwapchain, nullptr);
		hasWindow = false;
	}
}

void Swapchain::destroySyncObjects(VkDevice &device)
{
	if (hasSyncObjects)
	{
		for (size_t i = 0; i < imageCount; i++)
		{
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		hasSyncObjects = false;
	}
}

void Swapchain::destroy(VkDevice &device, VkInstance &instance)
{
	destroyVkSwapchain(device);
	destroySyncObjects(device);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	window->destroy();
}

void ApiContext::destroy()
{
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}

}        // namespace vka
*/
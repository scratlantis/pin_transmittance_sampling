#pragma once
#include "setup.h"
#include "../initializers/misc.h"

namespace vka
{


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

static void selectQueues(int universalQueueCount, int computeQueueCount, int &universalQueueFamily, int &computeQueueFamily)
{
	ASSERT_TRUE(state.initBits & (STATE_INIT_DEVICE_PHYSICAL_BIT | STATE_INIT_IO_WINDOW_BIT));
	universalQueueFamily = -1;
	computeQueueFamily   = -1;
	QueueFamilyIndices indices{};
	uint32_t           queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(state.device.physical, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(state.device.physical, &queueFamilyCount, queueFamilyList.data());
	int index = 0;
	for (auto &queueFamily : queueFamilyList)
	{
		if (universalQueueFamily = -1 &&
			queueFamily.queueCount >= universalQueueCount
			&& queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
		{
			VkBool32 presentation_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(state.device.physical, index, state.io.surface, &presentation_support);
			if (presentation_support)
			{
				universalQueueFamily = index;
				queueFamily.queueCount -= universalQueueCount;
			}
		}
		if (computeQueueFamily = -1 &&
			queueFamily.queueCount >= computeQueueCount
			&& queueFamily.queueFlags & (VK_QUEUE_COMPUTE_BIT))
		{
			computeQueueFamily = index;
			queueFamily.queueCount -= computeQueueCount;
		}
		if ((universalQueueFamily != -1 || universalQueueCount == 0) && (computeQueueFamily != -1 || computeQueueCount == 0))
		{
			break;
		}
	}
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


void Device::createInstance()
{
	VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
	appInfo.pApplicationName = deviceCI.applicationName.c_str();
	appInfo.pEngineName      = deviceCI.applicationName.c_str();
	appInfo.apiVersion       = API_VERSION;

	std::vector<const char *> instanceExtensions;
	if (deviceCI.enabledInstanceExtensions.size() > 0)
	{
		for (const char *enabledExtension : deviceCI.enabledInstanceExtensions)
		{
			if (std::find(deviceCI.enabledInstanceExtensions.begin(), deviceCI.enabledInstanceExtensions.end(), enabledExtension) == deviceCI.enabledInstanceExtensions.end())
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
	state.initBits |= STATE_INIT_DEVICE_INSTANCE_BIT;
}
void Device::selectPhysicalDevice()
{
	ASSERT_TRUE(state.initBits & (STATE_INIT_DEVICE_INSTANCE_BIT | STATE_INIT_IO_WINDOW_BIT));
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
		int devicePriority = checkDeviceSuitable(device, state.io.surface, deviceCI.enabledDeviceExtensions);
		if (devicePriority > maxDevicePriority)
		{
			physical    = device;
			maxDevicePriority = devicePriority;
		}
	}
	if (maxDevicePriority == 0)
	{
		throw std::runtime_error("Can not find suitable GPU!");
	}
	state.initBits |= STATE_INIT_DEVICE_PHYSICAL_BIT;
}
void Device::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCIs;
	VkDeviceQueueCreateInfo              queueCI{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};

	std::vector<float> piority(std::max(deviceCI.universalQueueCount, deviceCI.computeQueueCount));
	for (size_t i = 0; i < piority.size(); i++)
	{
		piority[i] = 1.0f;
	}
	queueCI.pQueuePriorities = piority.data();
	int universalFamily, computeFamily;
	selectQueues(deviceCI.universalQueueCount, deviceCI.computeQueueCount, universalFamily, computeFamily);

	if (deviceCI.universalQueueCount > 0)
	{
		queueCI.queueFamilyIndex = universalFamily;
		queueCI.queueCount       = deviceCI.universalQueueCount;
		queueCIs.push_back(queueCI);
	}
	if (deviceCI.computeQueueCount > 0)
	{
		queueCI.queueFamilyIndex = computeFamily;
		queueCI.queueCount       = deviceCI.computeQueueCount;
		queueCIs.push_back(queueCI);
	}

	VkDeviceCreateInfo logicalDeviceCI{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	logicalDeviceCI.queueCreateInfoCount    = VKA_COUNT(queueCIs);
	logicalDeviceCI.pQueueCreateInfos       = queueCIs.data();
	logicalDeviceCI.enabledExtensionCount   = VKA_COUNT(deviceCI.enabledDeviceExtensions);
	logicalDeviceCI.ppEnabledExtensionNames = deviceCI.enabledDeviceExtensions.data();
	logicalDeviceCI.pNext                   = deviceCI.enabledFeatures.chainNodes();

	ASSERT_VULKAN(vkCreateDevice(physical, &logicalDeviceCI, nullptr, &logical));

	universalQueues.resize(deviceCI.universalQueueCount);
	for (size_t i = 0; i < deviceCI.universalQueueCount; i++)
	{
		vkGetDeviceQueue(logical, universalFamily, i, &universalQueues[i]);
	}
	computeQueues.resize(deviceCI.computeQueueCount);
	if (computeFamily != universalFamily)
	{
		for (size_t i = 0; i < deviceCI.computeQueueCount; i++)
		{
			vkGetDeviceQueue(logical, computeFamily, i, &computeQueues[i]);
		}
	}
	else
	{
		for (size_t i = deviceCI.universalQueueCount; i < deviceCI.universalQueueCount + deviceCI.computeQueueCount; i++)
		{
			vkGetDeviceQueue(logical, computeFamily, i, &computeQueues[i]);
		}
	}
	state.initBits |= STATE_INIT_DEVICE_LOGICAL_BIT;
	state.initBits |= STATE_INIT_DEVICE_BIT;
}

WindowCI vka::IOControlerCI::getWindowCI()
{
	WindowCI windowCI{};
	windowCI.cursorMode = cursorMode;
	windowCI.title      = windowTitel;
	windowCI.height     = size.height;
	windowCI.width      = size.width;
	windowCI.resizable  = resizable;
	return windowCI;
}

void vka::IOController::configure(IOControlerCI &controllerCI, Window *window)
{
	ASSERT_TRUE(state.initBits & STATE_INIT_DEVICE_INSTANCE_BIT);
	this->controllerCI = controllerCI;
	this->window = window;
	window->init(controllerCI.getWindowCI(), state.device.instance);
	surface = window->getSurface();
	state.initBits |= STATE_INIT_IO_WINDOW_BIT;
}



void vka::IOController::init()
{
	ASSERT_TRUE(state.initBits & STATE_INIT_DEVICE_BIT);
	swapChainDetails = getSwapchainDetails(state.device.physical, window->getSurface());
	surfaceFormat    = swapChainDetails.formats[0];        // Must exist after vulkan specifications
	selectByPreference(swapChainDetails.formats, controllerCI.preferedFormats, surfaceFormat);
	format      = surfaceFormat.format;
	presentMode = VK_PRESENT_MODE_FIFO_KHR;
	selectByPreference(swapChainDetails.presentationMode, controllerCI.preferedPresentModes, presentMode);

	imageCount = controllerCI.preferedSwapchainImageCount;
	imageCount          = std::max(imageCount, swapChainDetails.surfaceCapabilities.minImageCount);
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0)        // 0 == limitless
	{
		imageCount = std::min(imageCount, swapChainDetails.surfaceCapabilities.maxImageCount);
	}
	shouldRecreateSwapchain = true;
	updateSwapchain();
}

void vka::IOController::updateSwapchain()
{
	if (!shouldRecreateSwapchain)
	{
		return;
	}
	while(window->size().height == 0 || window->size().width == 0)
	{
		window->waitEvents();
	}
	ASSERT_TRUE(state.initBits & STATE_INIT_DEVICE_BIT);
	vkDeviceWaitIdle(state.device.logical);

	extent = window->size();
	clamp(extent, swapChainDetails.surfaceCapabilities.minImageExtent, swapChainDetails.surfaceCapabilities.maxImageExtent);

	VkSwapchainCreateInfoKHR vkSwapchainCI{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	vkSwapchainCI.surface          = surface;
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

	QueueFamilyIndices indices = getQueueFamilies(state.device.physical, surface);

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

	VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
	if ((state.initBits & STATE_INIT_IO_SWAPCHAIN_BIT))
	{
		oldSwapchain = swapchain;
	}
	vkSwapchainCI.oldSwapchain = oldSwapchain;

	ASSERT_VULKAN(vkCreateSwapchainKHR(state.device.logical, &vkSwapchainCI, nullptr, &swapchain));

	if ((state.initBits & STATE_INIT_IO_SWAPCHAIN_BIT))
	{
		for (size_t i = 0; i < imageCount; i++)
		{
			vkDestroyImageView(state.device.logical, imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(state.device.logical, oldSwapchain, nullptr);
	}

	std::vector<VkImage> images(imageCount);
	imageViews.resize(imageCount);
	vkGetSwapchainImagesKHR(state.device.logical, swapchain, &imageCount, images.data());
	
	for (size_t i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo imageViewCI = ImageViewCreateInfo_Swapchain(images[i], format);
		ASSERT_VULKAN(vkCreateImageView(state.device.logical, &imageViewCI, nullptr, &imageViews[i]));
	}

	state.initBits |= STATE_INIT_IO_SWAPCHAIN_BIT;
	shouldRecreateSwapchain = false;
}
void vka::IOController::requestSwapchainRecreation()
{
	shouldRecreateSwapchain = true;
}

void vka::IOController::readInputs()
{
	memset(&keyEvent, 0, KEY_COUNT * sizeof(bool));
	mouse.resetEvents();
	window->pollEvents();
}

void State::init(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window *window)
{
	initBits                                   = 0;
	window->initWindowManager();
	window->addInstanceExtensions(deviceCI.enabledInstanceExtensions);
	device.configure(deviceCI);
	device.createInstance();
	io.configure(ioControllerCI, window);
	device.selectPhysicalDevice();
	device.createLogicalDevice();
	io.init();
	frame = nullptr;
	// RessourceTracker    heap;
	// RessourceTracker    cache;
	// MemAllocator        memAlloc;
	// DescriptorAllocator descAlloc;
	// QueryAllocator      queryAlloc;
	// CmdAllocator        cmdAlloc;
}
void State::destroy()
{
	//io.destroy();
	//device.destroy();
	//io.window->terminateWindowManager();
}

/*

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
*/

}        // namespace vka
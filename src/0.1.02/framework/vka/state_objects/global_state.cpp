#include "global_state.h"
#include <vka/interface/commands/commands.h> // civ
#include <vka/interface/cmd_buffer_functionality.h> // civ
#include <vka/resource_objects/ResourcePool.h>
#include <vka/resource_objects/ResourceCache.h>

namespace vka
{
Device::Device()
{
}
void Device::configure(DeviceCI &deviceCI)
{
	this->deviceCI = deviceCI;
}

void Device::destroy()
{
	vkDestroyDevice(logical, nullptr);
	vkDestroyInstance(instance, nullptr);
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
//		IF_VALIDATION(
		    instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//)
		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	const char *validationLayerName        = "VK_LAYER_KHRONOS_validation";
	instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
	instanceCreateInfo.enabledLayerCount   = 1;
	VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
	//IF_VALIDATION(
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
	    }
	gState.initBits |= STATE_INIT_DEVICE_INSTANCE_BIT;
}
void Device::selectPhysicalDevice()
{
	VKA_CHECK(gState.initBits & (STATE_INIT_DEVICE_INSTANCE_BIT | STATE_INIT_IO_WINDOW_BIT));
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
		int devicePriority = checkDeviceSuitable(device, gState.io.surface, deviceCI.enabledDeviceExtensions);
		if (devicePriority > maxDevicePriority)
		{
			physical          = device;
			maxDevicePriority = devicePriority;
		}
	}
	if (maxDevicePriority == 0)
	{
		throw std::runtime_error("Can not find suitable GPU!");
	}
	gState.initBits |= STATE_INIT_DEVICE_PHYSICAL_BIT;
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
	selectQueues(deviceCI.universalQueueCount, deviceCI.computeQueueCount, universalFamily, computeFamily, physical, gState.io.surface);

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

	VK_CHECK(vkCreateDevice(physical, &logicalDeviceCI, nullptr, &logical));

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
	gState.initBits |= STATE_INIT_DEVICE_LOGICAL_BIT;
	gState.initBits |= STATE_INIT_DEVICE_BIT;
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

IOController::IOController()
{
}

void vka::IOController::configure(IOControlerCI &controllerCI, Window *window)
{
	VKA_CHECK(gState.initBits & STATE_INIT_DEVICE_INSTANCE_BIT);
	this->controllerCI = controllerCI;
	this->window       = window;
	window->init(controllerCI.getWindowCI(), gState.device.instance);
	surface = window->getSurface();
	gState.initBits |= STATE_INIT_IO_WINDOW_BIT;
}

void vka::IOController::init()
{
	VKA_CHECK(gState.initBits & STATE_INIT_DEVICE_BIT);
	swapChainDetails = getSwapchainDetails(gState.device.physical, window->getSurface());
	surfaceFormat    = swapChainDetails.formats[0];        // Must exist after vulkan specifications
	selectByPreference(swapChainDetails.formats, controllerCI.preferedFormats, surfaceFormat);
	format      = surfaceFormat.format;
	presentMode = VK_PRESENT_MODE_FIFO_KHR;
	selectByPreference(swapChainDetails.presentationMode, controllerCI.preferedPresentModes, presentMode);

	imageCount = controllerCI.preferedSwapchainImageCount;
	imageCount = std::max(imageCount, swapChainDetails.surfaceCapabilities.minImageCount);
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0)        // 0 == limitless
	{
		imageCount = std::min(imageCount, swapChainDetails.surfaceCapabilities.maxImageCount);
	}
	shouldRecreateSwapchain = true;
	swapchainAttachmentPool = new ResourcePool();
	updateSwapchain();
	gState.initBits |= STATE_INIT_IO_BIT;
}

void vka::IOController::updateSwapchain()
{
	if (!shouldRecreateSwapchain)
	{
		swapchainWasRecreated = false;
		return;
	}
	vkDeviceWaitIdle(gState.device.logical);

	while (window->size().height == 0 || window->size().width == 0)
	{
		window->waitEvents();
	}
	swapChainDetails = getSwapchainDetails(gState.device.physical, window->getSurface());
	VKA_CHECK(gState.initBits & STATE_INIT_DEVICE_BIT);

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

	QueueFamilyIndices indices = getQueueFamilies(gState.device.physical, surface);

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
	if ((gState.initBits & STATE_INIT_IO_SWAPCHAIN_BIT))
	{
		oldSwapchain = swapchain;
	}
	vkSwapchainCI.oldSwapchain = oldSwapchain;

	VK_CHECK(vkCreateSwapchainKHR(gState.device.logical, &vkSwapchainCI, nullptr, &swapchain));

	if ((gState.initBits & (STATE_INIT_IO_SWAPCHAIN_BIT)))
	{
		VKA_CHECK((gState.initBits & (STATE_INIT_FRAME_BIT)))
		for (size_t i = 0; i < imageCount; i++)
		{
			vkDestroyImageView(gState.device.logical, imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(gState.device.logical, oldSwapchain, nullptr);
	}

	images.resize(imageCount);
	imageViews.resize(imageCount);
	imageLayouts.resize(imageCount);
	for (size_t i = 0; i < imageLayouts.size(); i++)
	{
		imageLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	VK_CHECK(vkGetSwapchainImagesKHR(gState.device.logical, swapchain, &imageCount, images.data()));
	for (size_t i = 0; i < imageCount; i++)
	{
		ImageViewCreateInfo_Default viewCI = ImageViewCreateInfo_Default(images[i], format);
		VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &imageViews[i]));
	}
	gState.initBits |= STATE_INIT_IO_SWAPCHAIN_BIT;
	shouldRecreateSwapchain = false;
	swapchainWasRecreated   = true;
	if (gState.initBits & STATE_INIT_ALL_BIT)
	{
		VKA_IMMEDIATE(swapchainAttachmentPool->refreshImages(cmdBuf));
	}
}
bool IOController::swapchainRecreated()
{
	return swapchainWasRecreated;
}
Window* IOController::getWindow()
{
	return window;
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

void vka::IOController::destroy()
{
	swapchainAttachmentPool->clear();
	delete swapchainAttachmentPool;
	vkDestroySwapchainKHR(gState.device.logical, swapchain, nullptr);
	for (size_t i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(gState.device.logical, imageViews[i], nullptr);
	}
	vkDestroySurfaceKHR(gState.device.instance, surface, nullptr);
	window->destroy();
	if (swapchainImage)
	{
		delete swapchainImage;
	}
}

void vka::IOController::terminateWindowManager()
{
	window->terminateWindowManager();
}

bool IOController::shouldTerminate()
{
	return window->shouldClose();
}

void vka::AppState::initFrames()
{
	VKA_CHECK(initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
	VkSemaphoreCreateInfo semaphoreCI{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo     fenceCI{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	frames.resize(io.imageCount);
	for (size_t i = 0; i < io.imageCount; i++)
	{
		frames[i].frameIndex = i;
		frames[i].next       = &frames[NEXT_INDEX(i, io.imageCount)];
		frames[i].previous   = &frames[PREVIOUS_INDEX(i, io.imageCount)];
		frames[i].stack    = new ResourcePool();
		VK_CHECK(vkCreateSemaphore(device.logical, &semaphoreCI, nullptr, &frames[i].imageAvailableSemaphore));
		VK_CHECK(vkCreateSemaphore(device.logical, &semaphoreCI, nullptr, &frames[i].renderFinishedSemaphore));
		VK_CHECK(vkCreateFence(device.logical, &fenceCI, nullptr, &frames[i].inFlightFence));
	}
	frame = frames.data();
	initBits |= STATE_INIT_FRAME_SEMAPHORE_BIT;
	initBits |= STATE_INIT_FRAME_BIT;
}

void vka::AppState::destroyFrames()
{
	for (auto &frame : frames)
	{
		vkDestroySemaphore(device.logical, frame.imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device.logical, frame.renderFinishedSemaphore, nullptr);
		vkDestroyFence(device.logical, frame.inFlightFence, nullptr);
		frame.stack->clear();
		delete frame.stack;
	}
}

void AppState::nextFrame()
{
	frame = frame->next;
	VK_CHECK(vkWaitForFences(device.logical, 1, &frame->inFlightFence, VK_TRUE, UINT64_MAX));
	frame->stack->clear();
	io.readInputs();
	io.updateSwapchain();
	if (io.swapchainRecreated())
	{
		frame = &frames[0];
	}
	io.imageLayouts[frame->frameIndex] = VK_IMAGE_LAYOUT_UNDEFINED;
}

AppState::AppState()
{
}

void AppState::init(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window *window)
{
	initBits = 0;
	window->initWindowManager();
	window->addInstanceExtensions(deviceCI.enabledInstanceExtensions);
	device.configure(deviceCI);
	device.createInstance();
	io.configure(ioControllerCI, window);
	device.selectPhysicalDevice();
	device.createLogicalDevice();
	io.init();
	initFrames();
	memAlloc.init();
	cmdAlloc.init();
	cache = new ResourceCache();
	initBits |= STATE_INIT_ALL_BIT;
}
SubmitSynchronizationInfo AppState::acquireNextSwapchainImage()
{
	uint32_t imageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device.logical, io.swapchain, UINT64_MAX, frame->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));
	VK_CHECK(vkResetFences(device.logical, 1, &frame->inFlightFence));
	SubmitSynchronizationInfo syncInfo{};
	syncInfo.waitSemaphores   = {frame->imageAvailableSemaphore};
	syncInfo.waitDstStageMask = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	syncInfo.signalSemaphores = {frame->renderFinishedSemaphore};
	syncInfo.signalFence      = frame->inFlightFence;
	return syncInfo;
}

void AppState::presentFrame()
{
	VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores    = &frame->renderFinishedSemaphore;
	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains        = &io.swapchain;
	presentInfo.pImageIndices      = &frame->frameIndex;
	VK_CHECK(vkQueuePresentKHR(device.universalQueues[0], &presentInfo));
}

void AppState::destroy()
{
	vkDeviceWaitIdle(device.logical);

	io.destroy();
	destroyFrames();
	cache->clearAll();
	delete cache;
	cmdAlloc.destroy();
	memAlloc.destroy();
	device.destroy();
	io.terminateWindowManager();
}

}        // namespace vka
std::string gVkaShaderPath = std::string(FRAMEWORK_SRC_DIR) + "/vka/shaders/";
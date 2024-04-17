#pragma once
#include <vulkan/vulkan.h>
#include "../global_state.h"

namespace vka
{
struct ImageViewCreateInfo_Swapchain : public VkImageViewCreateInfo
{
	ImageViewCreateInfo_Swapchain(const VkImage &vkImage, const VkFormat &vkFormat);
};

inline ImageViewCreateInfo_Swapchain::ImageViewCreateInfo_Swapchain(const VkImage &vkImage, const VkFormat &vkFormat) :
    VkImageViewCreateInfo()
{
	sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	subresourceRange.baseMipLevel   = 0;
	subresourceRange.levelCount     = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount     = 1;
	image                           = vkImage;
	format                          = vkFormat;
	subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
}

	struct D3VKPTDeviceCI : DeviceCI
{
	    D3VKPTDeviceCI(std::string appName)
	{
		applicationName     = appName;
		universalQueueCount     = 1;
		computeQueueCount		= 0;

		// Instance Extensions
		enabledInstanceExtensions = {};

		// Device Extensions
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);

		// Device Features
		VkPhysicalDeviceVulkan11Features vulkan11Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
		vulkan11Features.shaderDrawParameters = VK_TRUE;
		enabledFeatures.addNode(vulkan11Features);


		/*VkPhysicalDevice16BitStorageFeatures storage16Bit{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES};
		storage16Bit.storageBuffer16BitAccess             = VK_TRUE;
		enabledFeatures.addNode(storage16Bit);*/

		VkPhysicalDeviceVulkan12Features features12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
		features12.bufferDeviceAddress             = VK_TRUE;
		features12.shaderFloat16                   = VK_TRUE;
		features12.descriptorBindingPartiallyBound = VK_TRUE;
		enabledFeatures.addNode(features12);

		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
		shaderAtomicFeatures.shaderBufferFloat32Atomics   = true;
		shaderAtomicFeatures.shaderBufferFloat32AtomicAdd = true;
		enabledFeatures.addNode(shaderAtomicFeatures);

		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
		rayQueryFeatures.rayQuery                            = VK_TRUE;
		enabledFeatures.addNode(rayQueryFeatures);

		VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader64Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT};
		shader64Features.shaderImageInt64Atomics                           = VK_TRUE;
		enabledFeatures.addNode(shader64Features);

		/*VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
		indexing_features.runtimeDescriptorArray                    = VK_TRUE;
		indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		indexing_features.descriptorBindingPartiallyBound           = VK_TRUE;
		indexing_features.runtimeDescriptorArray                    = VK_TRUE;
		enabledFeatures.addNode(indexing_features);*/

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.textureCompressionBC     = VK_TRUE;
		deviceFeatures.imageCubeArray           = VK_TRUE;
		deviceFeatures.depthClamp               = VK_FALSE;
		deviceFeatures.depthBiasClamp           = VK_TRUE;
		deviceFeatures.depthBounds              = VK_TRUE;
		deviceFeatures.fillModeNonSolid         = VK_TRUE;
		deviceFeatures.samplerAnisotropy        = VK_TRUE;
		deviceFeatures.samplerAnisotropy        = VK_TRUE;
		deviceFeatures.shaderInt64              = VK_TRUE;
		deviceFeatures.shaderInt16              = VK_TRUE;
		deviceFeatures.multiDrawIndirect        = VK_TRUE;
		VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
		features2.features = deviceFeatures;
		enabledFeatures.addNode(features2);


		/*VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES};
		buffer_device_address_features.bufferDeviceAddress              = VK_TRUE;
		buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;
		buffer_device_address_features.bufferDeviceAddressMultiDevice   = VK_FALSE;
		enabledFeatures.addNode(buffer_device_address_features);*/

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
		ray_tracing_pipeline_features.rayTracingPipeline = VK_TRUE;
		enabledFeatures.addNode(ray_tracing_pipeline_features);

		VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
		acceleration_structure_features.accelerationStructure                                 = VK_TRUE;
		acceleration_structure_features.accelerationStructureIndirectBuild                    = VK_FALSE;
		acceleration_structure_features.accelerationStructureHostCommands                     = VK_FALSE;
		acceleration_structure_features.descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE;
		enabledFeatures.addNode(acceleration_structure_features);
	}
};

struct DefaultIOControlerCI : IOControlerCI
{
	DefaultIOControlerCI(std::string title, uint32_t width, uint32_t height)
	{
		windowTitel                 = title;
		resizable                   = true;
		size                        = {width, height};
		cursorMode                  = VISIBLE;
		preferedFormats             = {{VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
		preferedPresentModes        = {VK_PRESENT_MODE_MAILBOX_KHR};
		preferedSwapchainImageCount = 3;
	}
};

struct RayTracingShaderGroupCreateInfo_Empty : VkRayTracingShaderGroupCreateInfoKHR
{
	RayTracingShaderGroupCreateInfo_Empty()
	{
		sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		anyHitShader       = VK_SHADER_UNUSED_KHR;
		closestHitShader   = VK_SHADER_UNUSED_KHR;
		generalShader      = VK_SHADER_UNUSED_KHR;
		intersectionShader = VK_SHADER_UNUSED_KHR;
		type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	}
};


} // namespace vka
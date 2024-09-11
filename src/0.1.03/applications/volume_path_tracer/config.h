#pragma once
#include <vka/vka.h>
#include <glm/gtc/matrix_transform.hpp>
static const std::string shaderPath = std::string(APP_SRC_DIR) + "/shaders/";

using namespace vka;
using namespace vka::pbr;

enum GuiCategories
{
	GENERAL = 0,
	PERLIN_NOISE_SETTINGS = 1,
	PIN_SETTINGS = 2,
	VISUALIZATION_SETTINGS = 3,
	METRICS = 4,
};

extern GVar     gvar_model;
extern GVar     gvar_perlin_scale0;
extern GVar     gvar_perlin_scale1;
extern GVar     gvar_perlin_frequency0;
extern GVar     gvar_perlin_frequency1;
extern GVar     gvar_perlin_falloff;
extern GVar     gvar_medium_albedo_r;
extern GVar     gvar_medium_albedo_g;
extern GVar     gvar_medium_albedo_b;
extern GVar     gvar_image_resolution;
extern GVar     gvar_pin_count;
extern GVar     gvar_pin_transmittance_value_count;
extern GVar     gvar_pin_count_per_grid_cell;
extern GVar     gvar_pin_grid_size;
extern GVar     gvar_cursor_pos_x;
extern GVar     gvar_cursor_pos_y;
extern GVar     gvar_cursor_pos_z;
extern GVar     gvar_cursor_dir_phi;
extern GVar     gvar_cursor_dir_theta;
extern GVar     gvar_min_pin_bounce;
extern GVar     gvar_max_bounce;
extern GVar     gvar_timing_left;
extern GVar     gvar_timing_right;
extern GVar     gvar_raymarche_step_size;
extern GVar     gvar_medium_x;
extern GVar     gvar_medium_y;
extern GVar     gvar_medium_z;
extern GVar     gvar_medium_rot_y;
extern GVar     gvar_medium_scale;





struct DefaultDeviceCI : DeviceCI
{
	DefaultDeviceCI(std::string appName)
	{
		applicationName     = appName;
		universalQueueCount = 1;
		computeQueueCount   = 0;

		// Instance Extensions
		enabledInstanceExtensions = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

		// Device Extensions
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

		// Device Features
		VkPhysicalDeviceVulkan11Features vulkan11Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
		vulkan11Features.shaderDrawParameters = VK_TRUE;
		enabledFeatures.addNode(vulkan11Features);

		VkPhysicalDeviceVulkan12Features features12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
		features12.bufferDeviceAddress             = VK_TRUE;
		features12.shaderFloat16                   = VK_TRUE;
		features12.descriptorBindingPartiallyBound = VK_TRUE;
		enabledFeatures.addNode(features12);

		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
		shaderAtomicFeatures.shaderBufferFloat32Atomics   = VK_TRUE;
		shaderAtomicFeatures.shaderBufferFloat32AtomicAdd = VK_TRUE;
		enabledFeatures.addNode(shaderAtomicFeatures);

		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
		rayQueryFeatures.rayQuery = VK_TRUE;
		enabledFeatures.addNode(rayQueryFeatures);

		VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader64Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT};
		shader64Features.shaderImageInt64Atomics = VK_TRUE;
		enabledFeatures.addNode(shader64Features);


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
		deviceFeatures.independentBlend         = VK_TRUE;
		deviceFeatures.geometryShader           = VK_TRUE;
		VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
		features2.features = deviceFeatures;
		enabledFeatures.addNode(features2);


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
		cursorMode                  = vka::WINDOW_CURSOR_MODE_VISIBLE;
		preferedFormats             = {{VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
		preferedPresentModes        = {VK_PRESENT_MODE_MAILBOX_KHR};
		preferedSwapchainImageCount = 3;
	}
};

struct DefaultAdvancedStateConfig : AdvancedStateConfig
{
	DefaultAdvancedStateConfig()
	{
		modelPath   = std::string(RESOURCE_BASE_DIR) + "/models/";
		texturePath = std::string(RESOURCE_BASE_DIR) + "/textures/";
		modelUsage = 0;
	}
};

struct DefaultFixedCameraCI : public vka::FixedCameraCI
{
	DefaultFixedCameraCI()
	{
		fixpoint    = glm::vec3(0.0f, 0.0f, 0.0f);
		distance    = 1.0;
		up          = glm::vec3(0.0f, 1.0f, 0.0f);
		yaw         = 90.f;
		pitch       = 0.0f;
		moveSpeed   = 0.2f;
		turnSpeed   = 0.25f;
		scrollSpeed = 0.1f;
	}
};

struct Params
{
	glm::mat4 initialMediumMatrix;
	glm::vec3 initialMediumAlbedo;
	glm::uint volumeResolution;
};

struct DefaultParams : public Params
{
	DefaultParams()
	{
		initialMediumMatrix  = glm::mat4(1.0f);
		initialMediumMatrix  = glm::translate(initialMediumMatrix, glm::vec3(-0.2,-0.2,-0.2));
		//initialMediumMatrix  = glm::translate(initialMediumMatrix, glm::vec3(-0.2,-0.2,-0.4));
		initialMediumMatrix  = glm::scale(initialMediumMatrix, glm::vec3(0.3));
		initialMediumAlbedo  = glm::vec3(1.0);
		volumeResolution = 64;
	}
};
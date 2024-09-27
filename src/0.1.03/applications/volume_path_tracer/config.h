#pragma once
#include <vka/vka.h>
#include <glm/gtc/matrix_transform.hpp>
static const std::string shaderPath = std::string(APP_SRC_DIR) + "/shaders/";
static const std::string configPath = std::string(CONFIG_DIR) + "/";

using namespace vka;
using namespace vka::pbr;

enum GuiCategories
{
	PATH_TRACING,
	MEDIUM,
	NOISE_SETTINGS,
	PIN_SETTINGS,
	VISUALIZATION_SETTINGS,
	METRICS,
};
#define NO_GUI -1

#define MAX_PLOT_POINTS 100
#define MAX_PLOTS 2

const float               viewMargin     = 0.001;

const Rect2D<float>       viewDimensions    = {0.3, viewMargin, 0.7 - 1.0 * viewMargin, 1.0 - 2.0 * viewMargin};
const Rect2D<float>       leftGuiDimensions = {0.f, viewDimensions.y, viewDimensions.x, 0.7};
const Rect2D<float>       topGuiDimensions  = {0, 0, 0.5, viewDimensions.y};
//const Rect2D<float>       bottomGuiDimensions = {0.3, 0.7, 0.7, 0.3};
const Rect2D<float> bottomGuiDimensions = {leftGuiDimensions.x, leftGuiDimensions.height, leftGuiDimensions.width, 1.0 - leftGuiDimensions.height};


extern std::vector<GVar *> gVars;
extern std::unordered_map<GVar, bool> gVarHasChanged;

extern GVar     gvar_model;
extern GVar     gvar_perlin_scale0;
extern GVar     gvar_perlin_scale1;
extern GVar     gvar_perlin_frequency0;
extern GVar     gvar_perlin_frequency1;
extern GVar     gvar_perlin_falloff;
extern GVar     gvar_medium_albedo;
extern GVar     gvar_image_resolution;
extern GVar     gvar_pin_count;
extern GVar     gvar_pin_transmittance_value_count;
extern GVar     gvar_pin_count_per_grid_cell;
extern GVar     gvar_pin_grid_size;
extern GVar     gvar_cursor_pos;
extern GVar     gvar_cursor_dir_phi;
extern GVar     gvar_cursor_dir_theta;
extern GVar     gvar_min_pin_bounce;
extern GVar     gvar_max_bounce;
extern GVar     gvar_timing_left;
extern GVar     gvar_timing_right;
extern GVar     gvar_raymarche_step_size;
extern GVar     gvar_medium_pos;
extern GVar     gvar_medium_rot_y;
extern GVar     gvar_medium_scale;
extern GVar     gvar_env_map;
extern GVar     gvar_fixed_seed;
extern GVar     gvar_seed;
extern GVar     gvar_medium_xray_line_segments;
extern GVar     gvar_pin_sample_location;
extern GVar     gvar_continuous_path_sampling;

// Screen cursor
extern GVar gvar_path_sampling_event;
extern GVar gvar_screen_cursor_pos;
extern GVar gvar_screen_cursor_enable;
extern GVar gvar_screen_cursor_seed;


// Camera
extern GVar     gvar_cam_fixpoint;
extern GVar     gvar_cam_up;
extern GVar     gvar_cam_distance;
extern GVar     gvar_cam_yaw;
extern GVar     gvar_cam_pitch;
extern GVar     gvar_cam_move_speed;
extern GVar     gvar_cam_turn_speed;
extern GVar     gvar_cam_scroll_speed;

// Config
extern GVar gvar_select_config;
extern GVar gvar_save_config;
extern GVar gvar_save_config_name;
extern GVar gvar_reload_config;



struct GuiConfig
{
	bool  showPlots;
	Image plot1;
	Image plot2;
};
extern GuiConfig guiConf;




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

struct DefaultFixedCameraState : public vka::FixedCameraState
{
	DefaultFixedCameraState()
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

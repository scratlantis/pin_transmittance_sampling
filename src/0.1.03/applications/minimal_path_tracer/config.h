#pragma once
#include <vka/vka.h>
#include <glm/gtc/matrix_transform.hpp>
static const std::string shaderPath = std::string(APP_SRC_DIR) + "/shaders/";

struct D3VKPTDeviceCI : vka::DeviceCI
{
	D3VKPTDeviceCI(std::string appName)
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

struct DefaultIOControlerCI : vka::IOControlerCI
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

struct DefaultAdvancedStateConfig : vka::AdvancedStateConfig
{
	DefaultAdvancedStateConfig()
	{
		modelPath = std::string(APP_SRC_DIR) + "/models/";
		texturePath = std::string(APP_SRC_DIR) + "/textures/";
		modelUsage = 0;
	}
};

struct FixedCameraCI_Default : public vka::FixedCameraCI
{
	FixedCameraCI_Default()
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

// Shader interface
using namespace vka;
typedef uint32_t  uint;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;
#include "shaders/interface.glsl"

static GLSLFrame defaultFrame(VkExtent2D extent, uint32_t frameIdx)
{
	GLSLFrame frame     = {extent.width, extent.height, frameIdx};
	frame.projection    = glm::perspective(glm::radians(60.0f), (float) extent.width / (float) extent.height, 0.1f, 500.0f);
	frame.invProjection = glm::inverse(frame.projection);
	return frame;
}

static GLSLView cameraView(FixedCamera cam)
{
	GLSLView view;
	view.mat    = cam.getViewMatrix();
	view.invMat = glm::inverse(view.mat);
	view.pos    = vec4(cam.getPosition(), 0.0);
	return view;
}

static GLSLParams guiParams(std::vector<GVar*> gv)
{
	GLSLParams params{};
	return params;
}

static void configShaderPrelude1(CmdBuffer cmdBuf, ComputeCmd &cmd, VkExtent2D extent, FixedCamera cam, uint32_t frameIdx, Buffer ubo_frame, Buffer ubo_view, Buffer ubo_params)
{
	GLSLFrame ptFrame = defaultFrame(extent, frameIdx);
	cmdWriteCopy(cmdBuf, ubo_frame, &ptFrame, sizeof(GLSLFrame));
	cmd.pushDescriptor(ubo_frame, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	GLSLView ptView = cameraView(cam);
	cmdWriteCopy(cmdBuf, ubo_view, &ptView, sizeof(GLSLView));
	cmd.pushDescriptor(ubo_view, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	cmd.pushDescriptor(ubo_params, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

static void configShaderPrelude1(CmdBuffer cmdBuf, DrawCmd &cmd, VkExtent2D extent, FixedCamera cam, uint32_t frameIdx, Buffer ubo_frame, Buffer ubo_view, Buffer ubo_params)
{
	GLSLFrame ptFrame = defaultFrame(extent, frameIdx);
	cmdWriteCopy(cmdBuf, ubo_frame, &ptFrame, sizeof(GLSLFrame));
	cmd.pushDescriptor(ubo_frame, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	GLSLView ptView = cameraView(cam);
	cmdWriteCopy(cmdBuf, ubo_view, &ptView, sizeof(GLSLView));
	cmd.pushDescriptor(ubo_view, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	cmd.pushDescriptor(ubo_params, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

static void configShaderPrelude2(CmdBuffer cmdBuf, ComputeCmd &cmd, VkExtent2D extent, FixedCamera cam, uint32_t frameIdx, Buffer ubo_frame, Buffer ubo_view, Buffer ubo_params)
{
	GLSLFrame ptFrame = defaultFrame(extent, frameIdx);
	cmdWriteCopy(cmdBuf, ubo_frame, &ptFrame, sizeof(GLSLFrame));
	cmd.pushDescriptor(ubo_frame, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	cmd.pushDescriptor(ubo_params, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

static void configShaderPrelude2(CmdBuffer cmdBuf, DrawCmd &cmd, VkExtent2D extent, FixedCamera cam, uint32_t frameIdx, Buffer ubo_frame, Buffer ubo_view, Buffer ubo_params)
{
	GLSLFrame ptFrame = defaultFrame(extent, frameIdx);
	cmdWriteCopy(cmdBuf, ubo_frame, &ptFrame, sizeof(GLSLFrame));
	cmd.pushDescriptor(ubo_frame, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	cmd.pushDescriptor(ubo_params, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}
#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

#ifndef SHADER_SRC_DIR
#	define SHADER_SRC_DIR ""
#endif
#ifndef SHADER_SPV_DIR
#	define SHADER_SPV_DIR ""
#endif
#ifndef SHADER_LOG_DIR
#	define SHADER_LOG_DIR ""
#endif
#ifndef SHADER_SRC_CNT
#	define SHADER_SRC_CNT 0
#endif

#ifndef API_VERSION
#	define API_VERSION VK_API_VERSION_1_3
#endif        // !1

#ifndef ENGINE_NAME
#	define ENGINE_NAME "VkBase"
#endif        // !1

#ifndef ENGINE_VERSION
#	define ENGINE_VERSION VK_MAKE_VERSION(1, 0, 3)
#endif        // !1

#define VALIDATION true

#ifdef _WIN32
#	define GLSLC_COMMAND "glslc.exe --target-env=vulkan1.2"
#else        // not _WIN32
#	define GLSLC_COMMAND "glslc --target-env vulkan1.2"
#endif        // _WIN32

#define INTEL_VENDOR_ID 8086
#define AMD_VENDOR_ID 0x1002
#define NVIDIA_VENDOR_ID 0x10de

#ifdef _WIN32
#define FILE_SEPARATOR "\\"


#else // not _WIN32
#include <signal.h>
#define FILE_SEPARATOR '/'
#define DEBUG_BREAK raise(SIGTRAP);

#endif // _WIN32



// Get function pointers for vulkan commands from extensions.
// Makes function available as p<function name>
#define LOAD_CMD_VK_DEVICE(FUNCTION_NAME) PFN_##FUNCTION_NAME p##FUNCTION_NAME = (PFN_##FUNCTION_NAME) vkGetDeviceProcAddr(device.logical, #FUNCTION_NAME);
#define LOAD_CMD_VK_INSTANCE(FUNCTION_NAME) PFN_##FUNCTION_NAME p##FUNCTION_NAME = (PFN_##FUNCTION_NAME) vkGetInstanceProcAddr(device.instance, #FUNCTION_NAME);


#define DELETE_COPY_CONSTRUCTORS(A) A(const A&) = delete;\
									A(A&&) = delete;\
									A& operator=(const A&) = delete;\
									A& operator=(A&&) = delete;


#define VKA_COUNT(container) static_cast<uint32_t>((container).size())

#ifdef _MSC_VER
#	define DEBUG_BREAK __debugbreak();
#else
#	define DEBUG_BREAKDebugBreak() ;
#endif

#ifndef VALIDATION
#	define VALIDATION true
#endif        // !VALIDATION

#ifdef _DEBUG
// In debug mode validation should always be on
#	undef VALIDATION
#	define VALIDATION true
#endif

static std::string errorString(VkResult errorCode)
{
	switch (errorCode)
	{
#define STR(r)   \
	case VK_##r: \
		return #r
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
#undef STR
		default:
			return "UNKNOWN_ERROR";
	}
}

#if VALIDATION
#	define ASSERT_VULKAN(val)                                                                                                        \
		{                                                                                                                             \
			VkResult res = (val);                                                                                                     \
			if (res != VK_SUCCESS)                                                                                                    \
			{                                                                                                                         \
				std::cout << "Fatal : VkResult is \"" << errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
				DEBUG_BREAK                                                                                                           \
			}                                                                                                                         \
		}
#	define ASSERT_TRUE(val)                                                                             \
		{                                                                                                \
			if (!(val))                                                                                  \
			{                                                                                            \
				std::cout << "Fatal : Assert failed in " << __FILE__ << " at line " << __LINE__ << "\n"; \
				DEBUG_BREAK                                                                              \
			}                                                                                            \
		}
#	define IF_VALIDATION(f) f
#else
#	define IF_VALIDATION(f)
#	define ASSERT_VULKAN(val) val
#	define ASSERT_TRUE(val) val
#endif        // _DEBUG

#define NEXT_INDEX(x, y) ((x + 1) % (y))
#define PREVIOUS_INDEX(x, y) (((x + y - 1) % y))
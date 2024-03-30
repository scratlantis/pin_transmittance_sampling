#pragma once

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
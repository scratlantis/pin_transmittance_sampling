#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <vector>
#include <unordered_set>

namespace vka
{
class ApiObject
{
  public:
	~ApiObject();
	ApiObject();

	virtual VkObjectType getType() const noexcept = 0;

  private:
};
inline vka::ApiObject::~ApiObject()
{
}
inline inline vka::ApiObject::ApiObject()
{
}

template<class T>
class TApiObject : public ApiObject
{
  public:
	~TApiObject();
	TApiObject(T aHandle)
	{
		handle = aHandle;
	}

	T getHandle()
	{
		return handle;
	}

  private:
	T handle;
};
template <class T>
inline vka::TApiObject<T>::~TApiObject()
{
}



#define DEFINE_API_OBJECT(Handle, Type)												 \
	template <>                                                                      \
	class TApiObject<Handle>														 \
	{                                                                                \
	  public:                                                                        \
		VkObjectType getType() const noexcept                                        \
		{                                                                            \
			return Type;                                                             \
		}                                                                            \
	};                                                                               \
	typedef TApiObject<Handle> Handle##_ApiObj;



// vulkan abstraction:
// null -> application context -> state / (heap, garbage list, descriptor list, caches, )
// state -> allocators
// state -> cachable -> caches
// state -> memory -> heap, garbage list
// state -> scene -> global ressource bindings, scene graph, binding sets
// state, scene -> descriptors -> descriptor list
// state -> command buffers
// state -> utility




// device
DEFINE_API_OBJECT(VkPhysicalDevice, VK_OBJECT_TYPE_PHYSICAL_DEVICE)
DEFINE_API_OBJECT(VkQueue, VK_OBJECT_TYPE_QUEUE)
DEFINE_API_OBJECT(VkInstance, VK_OBJECT_TYPE_INSTANCE)
DEFINE_API_OBJECT(VkDevice, VK_OBJECT_TYPE_DEVICE)
class Device;
// presentation
DEFINE_API_OBJECT(VkSwapchainKHR, VK_OBJECT_TYPE_SWAPCHAIN_KHR)
DEFINE_API_OBJECT(VkSurfaceKHR, VK_OBJECT_TYPE_SURFACE_KHR)
struct Swapchain;
DEFINE_API_OBJECT(GLFWwindow, VK_OBJECT_TYPE_UNKNOWN)

struct WindowCI;
class Window;
class Input;
class IOController;
// presentation



// Cachables
DEFINE_API_OBJECT(VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
class PipelineLayout;
DEFINE_API_OBJECT(VkPipeline, VK_OBJECT_TYPE_PIPELINE)
class Pipeline;
DEFINE_API_OBJECT(VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
class DescriptorSetLayout;
DEFINE_API_OBJECT(VkSampler, VK_OBJECT_TYPE_SAMPLER)
class Sampler;
class Shader;


// Primary Ressources
DEFINE_API_OBJECT(VkBuffer, VK_OBJECT_TYPE_BUFFER)
DEFINE_API_OBJECT(VkImage, VK_OBJECT_TYPE_IMAGE)
DEFINE_API_OBJECT(VkDeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY)
DEFINE_API_OBJECT(VkBufferView, VK_OBJECT_TYPE_BUFFER_VIEW)
DEFINE_API_OBJECT(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW)
DEFINE_API_OBJECT(VmaAllocation, VK_OBJECT_TYPE_UNKNOWN)
DEFINE_API_OBJECT(VkAccelerationStructureKHR, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR)

// Deletable, hashable, comparable
class Ressource;


// Abstracted Ressources
class Buffer;
class Image;
class AccelerationStructure;
// Resource Tracking
typedef std::unordered_set<Ressource> RessourceTracker;

// command interface
DEFINE_API_OBJECT(VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER)
class CmdBuf;

// automatic update abstraction
DEFINE_API_OBJECT(VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
class DescriptorSet;

// Allocator abstraction
DEFINE_API_OBJECT(VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL)
class QueryAllocator;
DEFINE_API_OBJECT(VkDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL)
class DescriptorAllocator;
DEFINE_API_OBJECT(VkCommandPool, VK_OBJECT_TYPE_COMMAND_POOL)
class CmdAllocator;
DEFINE_API_OBJECT(VmaAllocator, VK_OBJECT_TYPE_UNKNOWN)
class MemAllocator;



// No abstraction
// but helper functions
DEFINE_API_OBJECT(VkRenderPass, VK_OBJECT_TYPE_RENDER_PASS)
DEFINE_API_OBJECT(VkFramebuffer, VK_OBJECT_TYPE_FRAMEBUFFER)
DEFINE_API_OBJECT(VkShaderModule, VK_OBJECT_TYPE_SHADER_MODULE)
DEFINE_API_OBJECT(VkPipelineCache, VK_OBJECT_TYPE_PIPELINE_CACHE)
DEFINE_API_OBJECT(VkSemaphore, VK_OBJECT_TYPE_SEMAPHORE)
DEFINE_API_OBJECT(VkFence, VK_OBJECT_TYPE_FENCE)
DEFINE_API_OBJECT(VkEvent, VK_OBJECT_TYPE_EVENT)


// Global State
struct Frame;
class AppState;



}        // namespace vka


// Next Level Abstraction:
// Ressources:
// Buffer
// Image
// AccelerationStructure
// CommandBuffer
// DescriptorSet

// Non Ressources:
// Allocators
// Caches
// Heap
// GarbageList
// Swapchain
// Frames
// Queues
// ApiContext



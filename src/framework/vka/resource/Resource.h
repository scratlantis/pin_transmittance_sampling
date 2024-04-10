#pragma once
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>
#include "../global_state.h"
namespace vka
{
class Resource
{

  protected:
	virtual bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		return true;
	}

  public:
	virtual size_t hash() const = 0;
	virtual void   free() = 0;

	bool operator==(Resource const &other) const
	{
		return this->_equals(other);
	}
};

class UniqueResource : public Resource
{
  public:
	UniqueResource();
	~UniqueResource();

  private:
};

UniqueResource::UniqueResource()
{
}

UniqueResource::~UniqueResource()
{
}

class NonUniqueResource : public Resource
{
  protected:
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<NonUniqueResource const &>(other);
			return this->getHandle() == other_.getHandle();
		}
	}
  public:
	~NonUniqueResource();

	virtual uint64_t  getHandle() const = 0;
	virtual void      free() = 0;
	size_t    hash() const
	{
		return static_cast<size_t>(getHandle());
	}

  private:
};

NonUniqueResource::NonUniqueResource()
{
}

NonUniqueResource::~NonUniqueResource()
{
}


#define DEFINE_RESOURCE_HANDLE(Handle, freeCall)				  \
class Handle##_R : public NonUniqueResource					  \
{																  \
public:															  \
	Handle##_R(Handle handle)									\
		{																  \
	this->handle = handle;										  \
}																  \
virtual uint64_t getHandle() const								  \
{																  \
	return (uint64_t) this->handle;								  \
}																  \
void free()														  \
{																  \
	freeCall;                              \
}																  \
private:														  \
	Handle handle;												  \
};

#define DEFINE_MEMORY_RESOURCE_HANDLE_VMA(Handle, freeCall) \
	class Handle##_MR_VMA : public NonUniqueResource  \
	{                                              \
	  public:                                      \
		Handle##_MR_VMA(Handle handle, VmaAllocation vmaAllocation)              \
		{                                          \
			this->handle = handle;                 \
			this->vmaAllocation = vmaAllocation;    \
		}                                          \
		virtual uint64_t getHandle() const         \
		{                                          \
			return (uint64_t) this->handle;        \
		}                                          \
		void free()                                \
		{                                          \
			freeCall;                              \
		}                                          \
                                                   \
	  private:                                     \
		Handle handle;                             \
		VmaAllocation vmaAllocation;                   \
	};

#define DEFINE_MEMORY_RESOURCE_HANDLE_VK(Handle, freeCall) \
	class Handle##_MR_VK : public NonUniqueResource         \
	{                                                       \
	  public:                                               \
		Handle##_MR_VK(Handle handle, VkDeviceMemory deviceMemory)                       \
		{                                                   \
			this->handle = handle;                          \
			this->deviceMemory = deviceMemory;              \
		}                                                   \
		virtual uint64_t getHandle() const                  \
		{                                                   \
			return (uint64_t) this->handle;                 \
		}                                                   \
		void free()                                         \
		{                                                   \
			freeCall;                                       \
		}                                                   \
                                                            \
	  private:                                              \
		Handle        handle;                               \
		VkDeviceMemory deviceMemory;                        \
	};


struct DescriptorSetLayoutDefinition;
class DescriptorSetLayout : public UniqueResource
{

};

struct PipelineLayoutDefinition;
class PipelineLayout;

struct PipelineState;
class Pipeline;

struct SamplerDescription;
class Sampler;

struct ShaderDefinition;
class Shader;




// Primary Ressources
DEFINE_MEMORY_RESOURCE_HANDLE_VK(VkBuffer, gState.memAlloc.destroyBuffer(handle, deviceMemory))
DEFINE_MEMORY_RESOURCE_HANDLE_VMA(VkBuffer, gState.memAlloc.destroyBuffer(handle, vmaAllocation))
DEFINE_RESOURCE_HANDLE(VkBufferView, vkDestroyBufferView(gState.device.logical, handle, nullptr))

DEFINE_MEMORY_RESOURCE_HANDLE_VK(VkImage, gState.memAlloc.destroyImage(handle, deviceMemory))
DEFINE_MEMORY_RESOURCE_HANDLE_VMA(VkImage, gState.memAlloc.destroyImage(handle, vmaAllocation))
DEFINE_RESOURCE_HANDLE(VkImageView, vkDestroyImageView(gState.device.logical, handle, nullptr))

DEFINE_RESOURCE_HANDLE(VkAccelerationStructureKHR, vkDestroyAccelerationStructureKHR(gState.device.logical, handle, nullptr))







}        // namespace vka
namespace std
{
template <>
struct hash<vka::Resource>
{
	size_t operator()(vka::Resource const &r) const
	{
		return r.hash();
	}
};
}        // namespace std
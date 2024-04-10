#pragma once
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>
#include "../global_state.h"



namespace vka
{
template<class T>
class UniqueResource : public Resource
{
	protected:
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			return this->_equals(other);
		}
	}


	virtual void     free()            = 0;
	T            handle;
  public:
	virtual hash_t   _hash() const      = 0;

	~UniqueResource(){};

	T getHandle()
	{
		if (handle == VK_NULL_HANDLE)
		{
			Resource *result = gState.cache.find(this);
			if (result)
			{
				T *d   = dynamic_cast<T *>(result);
				handle = d->getHandle();
			}
			else
			{
				T *d = new T(definition);
				d->buildHandle();
				handle = d->handle;
				gState.cache.add(d);
			}
		}
		return handle;
	}
  private:
};


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
			return this->getId() == other_.getId();
		}
	}
	virtual void      free() = 0;

  public:
	~NonUniqueResource();

	hash_t            _hash() const
	{
		return static_cast<hash_t>(getId());
	}
	virtual uint64_t  getId() const = 0;

  private:
};

class MappableResource : public NonUniqueResource
{
  public:
	~MappableResource();
	virtual void *map(uint32_t offset, uint32_t size) = 0;
	virtual void unmap() = 0;
};





#define DEFINE_RESOURCE_HANDLE(Handle, freeCall)				  \
class Handle##_Resource : public NonUniqueResource					  \
{																  \
public:															  \
	Handle##_Resource(Handle handle)									\
		{																  \
	this->handle = handle;										  \
}																  \
virtual uint64_t getId() const                 \
		{																  \
	return (uint64_t) this->handle;								  \
}																  \
protected:\
void free()														  \
{																  \
	freeCall;                              \
}																  \
private:														  \
	Handle handle;												  \
};

#define DEFINE_MEMORY_RESOURCE_HANDLE_VMA(Handle, freeCall) \
	class Handle##_MemoryResource_VMA : public NonUniqueResource  \
	{                                              \
	  public:                                      \
		Handle##_MemoryResource_VMA(Handle handle, VmaAllocation vmaAllocation)              \
		{                                          \
			this->handle = handle;                 \
			this->vmaAllocation = vmaAllocation;    \
		}                                          \
		virtual uint64_t getId() const         \
		{                                          \
			return (uint64_t) this->handle;        \
		}                                          \
protected:\
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
	class Handle##_MemoryResource_VK : public NonUniqueResource         \
	{                                                       \
	  public:                                               \
		Handle##_MemoryResource_VK(Handle handle, VkDeviceMemory deviceMemory)                       \
		{                                                   \
			this->handle = handle;                          \
			this->deviceMemory = deviceMemory;              \
		}                                                   \
		virtual uint64_t getId() const                  \
		{                                                   \
			return (uint64_t) this->handle;                 \
		}                                                   \
protected:\
		void free()                                         \
		{                                                   \
			freeCall;                                       \
		}                                                   \
                                                            \
	  private:                                              \
		Handle        handle;                               \
		VkDeviceMemory deviceMemory;                        \
	};

class VkBuffer_MemoryResource_VK : public NonUniqueResource, public MappableResource
{
  public:
	VkBuffer_MemoryResource_VK(VkBuffer handle, VkDeviceMemory deviceMemory)
	{
		this->handle       = handle;
		this->deviceMemory = deviceMemory;
	}
	virtual uint64_t getId() const
	{
		return (uint64_t) this->handle;
	}
	void* map(uint32_t offset, uint32_t size)
	{
		void* data;
		gState.memAlloc.mapMemory(deviceMemory, offset, size, &data);
		return data;
	}
	void unmap()
	{
		gState.memAlloc.unmapMemory(deviceMemory);
	}
  protected:
	void free()
	{
		gState.memAlloc.destroyBuffer(handle, deviceMemory);
	}
  private:
	VkBuffer       handle;
	VkDeviceMemory deviceMemory;
};

class VkBuffer_MemoryResource_VMA : public NonUniqueResource, public MappableResource
{
  public:
	VkBuffer_MemoryResource_VMA(VkBuffer handle, VmaAllocation allocation)
	{
		this->handle       = handle;
		this->allocation = allocation;
	}
	virtual uint64_t getId() const
	{
		return (uint64_t) this->handle;
	}
	void *map(uint32_t offset, uint32_t size)
	{
		void *data;
		gState.memAlloc.mapMemory(allocation, &data);
		data = (char *)data + offset;
		return data;
	}
	void unmap()
	{
		gState.memAlloc.unmapMemory(allocation);
	}
  protected:
	void free()
	{
		gState.memAlloc.destroyBuffer(handle, allocation);
	}

  private:
	VkBuffer       handle;
	VmaAllocation allocation;
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

DEFINE_MEMORY_RESOURCE_HANDLE_VK(VkImage, gState.memAlloc.destroyImage(handle, deviceMemory))
DEFINE_MEMORY_RESOURCE_HANDLE_VMA(VkImage, gState.memAlloc.destroyImage(handle, vmaAllocation))

DEFINE_RESOURCE_HANDLE(VkImageView, vkDestroyImageView(gState.device.logical, handle, nullptr))
DEFINE_RESOURCE_HANDLE(VkBufferView, vkDestroyBufferView(gState.device.logical, handle, nullptr))

DEFINE_RESOURCE_HANDLE(VkAccelerationStructureKHR, vkDestroyAccelerationStructureKHR(gState.device.logical, handle, nullptr))







}        // namespace vka

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
	UniqueResource(ResourceTracker *pTracker)
		{
			this->pTracker = pTracker;
		    handle         = VK_NULL_HANDLE;
		}
	virtual void     free()            = 0;
	virtual void     buildHandle()     = 0;
	virtual UniqueResource<T>* copyToHeap() const      = 0;
	T            handle;
	ResourceTracker *pTracker;
  public:
	virtual hash_t   _hash() const      = 0;

	~UniqueResource(){};

	T getHandle()
	{
		if (handle == VK_NULL_HANDLE)
		{
			Resource *result = pTracker->find(this);
			if (result)
			{
				UniqueResource<T> *d = dynamic_cast<UniqueResource<T> *>(result);
				handle = d->getHandle();
			}
			else
			{
				UniqueResource<T> *d = this->copyToHeap();
				d->buildHandle();
				handle = d->handle;
				pTracker->add(d);
			}
		}
		return handle;
	}
  private:
};


class NonUniqueResource : public Resource
{
  protected:
	virtual void      free() = 0;
	virtual uint64_t  getId() const = 0;
  public:
	~NonUniqueResource();

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

	hash_t            _hash() const
	{
		return static_cast<hash_t>(getId());
	}

  private:
};

class MappableResource : public NonUniqueResource
{
  public:
	~MappableResource();
	virtual void *map(uint32_t offset, uint32_t size) = 0;
	virtual void unmap() = 0;
};



class BufferVK_R : public MappableResource
{
  public:
	BufferVK_R(VkBuffer handle, VkDeviceMemory deviceMemory)
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

class BufferVMA_R : public MappableResource
{
  public:
	BufferVMA_R(VkBuffer handle, VmaAllocation allocation)
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

class ImageVMA_R : public NonUniqueResource
{
  public:
	ImageVMA_R(VkImage handle, VmaAllocation allocation)
	{
		this->handle     = handle;
		this->allocation = allocation;
	}
	virtual uint64_t getId() const
	{
		return (uint64_t) this->handle;
	}
  protected:
	void free()
	{
		gState.memAlloc.destroyImage(handle, allocation);
	}

  private:
	VkImage       handle;
	VmaAllocation allocation;
};

class ImageVK_R : public NonUniqueResource
{
  public:
	ImageVK_R(VkImage handle, VkDeviceMemory deviceMemory)
	{
		this->handle     = handle;
		this->deviceMemory = deviceMemory;
	}
	virtual uint64_t getId() const
	{
		return (uint64_t) this->handle;
	}
  protected:
	void free()
	{
		gState.memAlloc.destroyImage(handle, deviceMemory);
	}

  private:
	VkImage       handle;
	VkDeviceMemory deviceMemory;
};

class ImageView_R : public NonUniqueResource
{
  public:
	ImageView_R(VkImageView handle)
	{
		this->handle       = handle;
	}
	virtual uint64_t getId() const
	{
		return (uint64_t) this->handle;
	}
  protected:
	void free()
	{
		vkDestroyImageView(gState.device.logical, handle, nullptr);
	}
  private:
	VkImageView    handle;
};

class BufferView_R : public NonUniqueResource
{
  public:
	BufferView_R(VkBufferView handle)
	{
		this->handle = handle;
	}
	virtual uint64_t getId() const
	{
		return (uint64_t) this->handle;
	}
  protected:
	void free()
	{
		vkDestroyBufferView(gState.device.logical, handle, nullptr);
	}

  private:
	VkBufferView handle;
};

class AccelerationStructure_R : public NonUniqueResource
{
  public:
	AccelerationStructure_R(VkAccelerationStructureKHR handle)
	{
		this->handle = handle;
	}
	virtual uint64_t getId() const
	{
		return (uint64_t) this->handle;
	}
  protected:
	void free()
	{
		vkDestroyAccelerationStructureKHR(gState.device.logical, handle, nullptr);
	}
  private:
	VkAccelerationStructureKHR handle;
};


struct ShaderDefinition;
class Shader;

struct PipelineState;
class Pipeline;

struct SamplerDescription;
class Sampler;








//#define DEFINE_RESOURCE_HANDLE(Handle, freeCall)				  \
//class Handle##_Resource : public NonUniqueResource					  \
//{																  \
//public:															  \
//	Handle##_Resource(Handle handle)									\
//		{																  \
//	this->handle = handle;										  \
//}																  \
//virtual uint64_t getId() const                 \
//		{																  \
//	return (uint64_t) this->handle;								  \
//}																  \
//protected:\
//void free()														  \
//{																  \
//	freeCall;                              \
//}																  \
//private:														  \
//	Handle handle;												  \
//};
//
// #define DEFINE_MEMORY_RESOURCE_HANDLE_VMA(Handle, freeCall) \
//	class Handle##_MemoryResource_VMA : public NonUniqueResource  \
//	{                                              \
//	  public:                                      \
//		Handle##_MemoryResource_VMA(Handle handle, VmaAllocation vmaAllocation)              \
//		{                                          \
//			this->handle = handle;                 \
//			this->vmaAllocation = vmaAllocation;    \
//		}                                          \
//		virtual uint64_t getId() const         \
//		{                                          \
//			return (uint64_t) this->handle;        \
//		}                                          \
//protected:\
//		void free()                                \
//		{                                          \
//			freeCall;                              \
//		}                                          \
//                                                   \
//	  private:                                     \
//		Handle handle;                             \
//		VmaAllocation vmaAllocation;                   \
//	};
//
// #define DEFINE_MEMORY_RESOURCE_HANDLE_VK(Handle, freeCall) \
//	class Handle##_MemoryResource_VK : public NonUniqueResource         \
//	{                                                       \
//	  public:                                               \
//		Handle##_MemoryResource_VK(Handle handle, VkDeviceMemory deviceMemory)                       \
//		{                                                   \
//			this->handle = handle;                          \
//			this->deviceMemory = deviceMemory;              \
//		}                                                   \
//		virtual uint64_t getId() const                  \
//		{                                                   \
//			return (uint64_t) this->handle;                 \
//		}                                                   \
//protected:\
//		void free()                                         \
//		{                                                   \
//			freeCall;                                       \
//		}                                                   \
//                                                            \
//	  private:                                              \
//		Handle        handle;                               \
//		VkDeviceMemory deviceMemory;                        \
//	};
// Primary Ressources

//DEFINE_MEMORY_RESOURCE_HANDLE_VK(VkImage, gState.memAlloc.destroyImage(handle, deviceMemory))
//DEFINE_MEMORY_RESOURCE_HANDLE_VMA(VkImage, gState.memAlloc.destroyImage(handle, vmaAllocation))
//DEFINE_RESOURCE_HANDLE(VkImageView, vkDestroyImageView(gState.device.logical, handle, nullptr))
//DEFINE_RESOURCE_HANDLE(VkBufferView, vkDestroyBufferView(gState.device.logical, handle, nullptr))
//DEFINE_RESOURCE_HANDLE(VkAccelerationStructureKHR, vkDestroyAccelerationStructureKHR(gState.device.logical, handle, nullptr))







}        // namespace vka

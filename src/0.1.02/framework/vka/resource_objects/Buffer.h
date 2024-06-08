#pragma
#include "Resource.h"
#include <vma/vk_mem_alloc.h>

namespace vka
{
// _I internal
// _R resource

class Buffer_R : public Resource_T<VkBuffer>
{
  public:
	Buffer_R(VkBuffer handle) :
	    Resource_T<VkBuffer>(handle){};
	virtual void *map(uint32_t offset, uint32_t size) = 0;
	virtual void  unmap()                             = 0;
};


class BufferVK_R : public Buffer_R
{
  public:
	BufferVK_R(VkBuffer handle, VkDeviceMemory deviceMemory) :
	    Buffer_R(handle) 
	{
		this->deviceMemory = deviceMemory;
	}
	void  free() override;
	void *map(uint32_t offset, uint32_t size) override;
	void  unmap() override;
  private:
	VkDeviceMemory deviceMemory;
};

class BufferVMA_R : public Buffer_R
{
  public:
	BufferVMA_R(VkBuffer handle, VmaAllocation allocation):
	    Buffer_R(handle) 
	{
		this->allocation = allocation;
	}

	void  free() override;
	void *map(uint32_t offset, uint32_t size) override;
	void  unmap() override;
  private:
	VmaAllocation allocation;
};

class BufferView_R : public Resource_T<VkBufferView>
{
  public:
	BufferView_R(VkBufferView handle):
	    Resource_T<VkBufferView>(handle){}
	void free() override;
};

enum class BufferType
{
	VMA,
	VK
};

union MemoryProperty
{
	VmaMemoryUsage        vma;
	VkMemoryPropertyFlags vk;
};


class Buffer_I : public Resource_T<VkBuffer>
{
  protected:
	Buffer_R          *res      = nullptr;
	BufferView_R      *viewRes  = nullptr;
	bool               isMapped        = false;
	bool               hasMemoryOwnership = false;
  public:
	VkBufferView       viewHandle = VK_NULL_HANDLE;

	// State
	BufferType         type;
	VkDeviceSize       size;
	VkBufferUsageFlags usage;
	MemoryProperty     memProperty;

	Buffer_I() : Resource_T<VkBuffer>(VK_NULL_HANDLE)
	{
		type               = BufferType::VMA;
		usage              = 0;
		memProperty.vk     = 0;
		size               = 0;
	};
	Buffer_I(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty) :
	    Buffer_I()
	{
		create(type, size, usage, memProperty);
	}
	~Buffer_I()
	{
		free();
	}
  private:
	Buffer_I &operator=(const Buffer_I &rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}
		// No ownership, no tracking
		res                = nullptr;
		viewRes            = nullptr;
		hasMemoryOwnership = false;
		isMapped           = false;
		pPool              = nullptr;

		handle     = rhs.handle;
		viewHandle = rhs.viewHandle;

		type        = rhs.type;
		size        = rhs.size;
		usage       = rhs.usage;
		memProperty = rhs.memProperty;
		return *this;
	}
  public:
	void   create(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty);
	Buffer_I recreate(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty, bool maintainData);
	void   track(IResourcePool *pPool) override;
	void   free() override;
	hash_t hash() const override;
	void  *map(uint32_t offset, uint32_t size);
	void   unmap();
};
}
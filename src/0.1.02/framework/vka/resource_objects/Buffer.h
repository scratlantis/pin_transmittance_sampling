#pragma
#include "Resource.h"
#include <vma/vk_mem_alloc.h>
#include "IDescriptor.h"

namespace vka
{
// _I internal
// _R resource

class Buffer_R : public Resource_T<VkBuffer>
{
  public:
	Buffer_R(VkBuffer handle) :
	    Resource_T<VkBuffer>(handle){};
	virtual void *map(uint32_t offset, uint32_t size) const = 0;
	virtual void  unmap() const                             = 0;
};


//class BufferVK_R : public Buffer_R
//{
//  public:
//	BufferVK_R(VkBuffer handle, VkDeviceMemory deviceMemory) :
//	    Buffer_R(handle) 
//	{
//		this->deviceMemory = deviceMemory;
//	}
//	void  free() override;
//	void *map(uint32_t offset, uint32_t size) override;
//	void  unmap() override;
//  private:
//	VkDeviceMemory deviceMemory;
//};

class BufferVMA_R : public Buffer_R
{
  public:
	BufferVMA_R(VkBuffer handle, VmaAllocation allocation):
	    Buffer_R(handle) 
	{
		this->allocation = allocation;
	}

	void  free() override;
	void *map(uint32_t offset, uint32_t size) const override;
	void  unmap() const override;
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
	NONE,
	VMA,
	VK
};

union MemoryProperty
{
	VmaMemoryUsage        vma;
	VkMemoryPropertyFlags vk;
};

struct BufferState
{
	BufferType         type;
	VkDeviceSize       size;
	VkBufferUsageFlags usage;
	MemoryProperty     memProperty;
};

class Buffer_I : public Resource_T<VkBuffer>, public IDescriptor
{
  protected:
	Buffer_R          *res      = nullptr;
	BufferView_R      *viewRes  = nullptr;
	bool               isMapped        = false;
	bool               hasMemoryOwnership = false;
  public:
	VkBufferView       viewHandle = VK_NULL_HANDLE;

	// State
	BufferState		state;
	BufferState		newState;


	Buffer_I(IResourcePool* pPool) : Resource_T<VkBuffer>(VK_NULL_HANDLE)
	{
		track(pPool);
		state.type            = BufferType::NONE;
		state.usage           = 0;
		state.memProperty.vma = VMA_MEMORY_USAGE_CPU_ONLY;
		state.size            = 0;
		newState = state;
	};

	Buffer_I() :
	    Buffer_I(nullptr){};

	Buffer_I(IResourcePool *pPool, VkBufferUsageFlags usage) :
	    Buffer_I(pPool)
	{
		newState.usage = usage;
	};
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
		res                = rhs.res;
		viewRes            = rhs.viewRes;
		pPool              = nullptr;

		handle     = rhs.handle;
		viewHandle = rhs.viewHandle;

		state.type        = rhs.state.type;
		state.size        = rhs.state.size;
		state.usage       = rhs.state.usage;
		state.memProperty = rhs.state.memProperty;
		return *this;
	}
  public:
	bool     isMappable() const;
	void   track(IResourcePool *pPool) override;
	hash_t hash() const override;
	void  *map(uint32_t offset, uint32_t size) const;
	void   unmap() const;

	void changeSize(VkDeviceSize size);
	void addUsage(VkBufferUsageFlags usage);
	void changeUsage(VkBufferUsageFlags usage);
	void changeMemoryType(VmaMemoryUsage memProperty);

	VkDeviceSize getSize() const;
	VkBufferUsageFlags getUsage() const;
	VmaMemoryUsage        getMemoryType() const;
	const vka::Buffer_R *getMappable() const;

	void           free(){};
	void createHandles();
	void detachChildResources();
	const Buffer_I recreate();
	void update();

	void writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const override;
};
}
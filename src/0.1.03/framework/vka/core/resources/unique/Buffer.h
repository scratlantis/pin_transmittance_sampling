#pragma once
#include "../Resource.h"
#include "../IDescriptor.h"
#include <vma/vk_mem_alloc.h>

namespace vka
{

class Mappable_T : public Resource_T<VkBuffer>
{
  public:
	Mappable_T(VkBuffer handle) :
	    Resource_T<VkBuffer>(handle){};
	virtual void *map(uint32_t offset, uint32_t size) const = 0;
	virtual void  unmap() const                             = 0;
};

class BufferVMA_R : public Mappable_T
{
  public:
	BufferVMA_R(VkBuffer handle, VmaAllocation allocation) :
	    Mappable_T(handle)
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
	BufferView_R(VkBufferView handle) :
	    Resource_T<VkBufferView>(handle)
	{}
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

struct BufferRange
{
	VkDeviceSize offset = 0;
	VkDeviceSize size = VK_WHOLE_SIZE;
};

class Buffer_R : public Resource_T<VkBuffer>
{
  protected:
	Mappable_T   *res                = nullptr;
	BufferView_R *viewRes            = nullptr;
	bool          isMapped           = false;

  public:
	VkBufferView viewHandle = VK_NULL_HANDLE;

	// State
	BufferState state;
	BufferState newState;

	// Range
	BufferRange range;

	Buffer_R(IResourcePool *pPool) :
	    Resource_T<VkBuffer>(VK_NULL_HANDLE)
	{
		track(pPool);
		state.type            = BufferType::NONE;
		state.usage           = 0;
		state.memProperty.vma = VMA_MEMORY_USAGE_CPU_ONLY;
		state.size            = 0;
		newState              = state;
	};

	Buffer_R() :
	    Buffer_R(nullptr){};

	Buffer_R(IResourcePool *pPool, VkBufferUsageFlags usage) :
	    Buffer_R(pPool)
	{
		newState.usage = usage;
	};
	~Buffer_R()
	{
		free();
	}

  private:
	Buffer_R &operator=(const Buffer_R &rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}
		res     = rhs.res;
		viewRes = rhs.viewRes;
		pPool   = nullptr;

		handle     = rhs.handle;
		viewHandle = rhs.viewHandle;

		state.type        = rhs.state.type;
		state.size        = rhs.state.size;
		state.usage       = rhs.state.usage;
		state.memProperty = rhs.state.memProperty;

		range.offset = rhs.range.offset;
		range.size   = rhs.range.size;

		return *this;
	}



  public:
	bool   isMappable() const;
	void   track(IResourcePool *pPool) override;
	hash_t hash() const override;
	void  *map(uint32_t offset, uint32_t size) const;
	void   unmap() const;

	void changeSize(VkDeviceSize size);
	void addUsage(VkBufferUsageFlags usage);
	void changeUsage(VkBufferUsageFlags usage);
	void changeMemoryType(VmaMemoryUsage memProperty);

	VkDeviceSize       getSize() const;
	VkBufferUsageFlags getUsage() const;
	VmaMemoryUsage     getMemoryType() const;

	BufferRange     getRange() const;
	const Buffer_R  getSubBuffer(BufferRange range) const;
	const Buffer_R  getShallowCopy() const;
	Buffer_R        getStagingBuffer() const;
	VkDeviceAddress getDeviceAddress() const;
	

	void           free(){};
	void           createHandles();
	void           detachChildResources();
	const Buffer_R recreate();
	void           update();
};
}        // namespace vka

typedef vka::Buffer_R* Buffer;
typedef const vka::Buffer_R* ImmutableBuffer;
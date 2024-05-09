#pragma once
#include "../resources/Resource.h"

namespace vka
{

class MemoryBlock
{
  protected:
	MappableResource *res = nullptr;
  public:
	MemoryBlock(){};
	MemoryBlock(ResourceTracker *pTracker, size_t size, bool fillZero = true)
	{
		this->size = size;
		if (fillZero)
		{
			data = std::calloc(size, 1);
		}
		else
		{
			data = std::malloc(size);
		}
		res = new BufferCPU_R(data, size);
		pTracker->add(res);
	}
	size_t getSize() const
	{
		return size;
	}

	void *getData() const
	{
		return data;
	}

	void writeData(const void *src, size_t dataSize)
	{
		memcpy(data, src, dataSize);
	}

	void move(ResourceTracker *pNewTracker)
	{
		ASSERT_TRUE(res != nullptr);
		res->move(pNewTracker);
	}

	~MemoryBlock(){};

  private:
	void *data = nullptr;
	size_t size = 0;
};




class Buffer
{
  protected:
	MappableResource *res     = nullptr;
	BufferView_R     *viewRes = nullptr;

  public:
	VkBuffer     buf  = VK_NULL_HANDLE;
	VkBufferView view = VK_NULL_HANDLE;
	VkDeviceSize size = 0;

	void *data = nullptr;        // used for mapping
	Buffer()   = default;

	uint32_t getOffset() const
	{
		return 0;
	}	

	VkDescriptorBufferInfo getDescriptorInfo() const
	{
		VkDescriptorBufferInfo info{};
		info.buffer = buf;
		info.offset = 0;
		info.range  = VK_WHOLE_SIZE;
		return info;
	}

	void move(ResourceTracker* pNewTracker)
	{
		ASSERT_TRUE(res != nullptr);
		res->move(pNewTracker);
		if (viewRes != nullptr)
		{
			viewRes->move(pNewTracker);
		}
	}

	/*void moveView(ResourceTracker *pNewTracker)
	{
		if (viewRes != nullptr)
		{
			viewRes->move(pNewTracker);
		}
	}*/

	void map(uint32_t offset = 0)
	{
		if (data == nullptr)
		{
			data = res->map(0, size);
		}
	}

	void unmap()
	{
		if (data != nullptr)
		{
			res->unmap();
			data = nullptr;
		}
	}

	void write(const void *src, VkDeviceSize dataSize, VkDeviceSize offset = 0)
	{
		map(offset);
		memcpy((char *) data, src, dataSize);
		unmap();
	}

	void write(const void *src)
	{
		map();
		memcpy((char *) data, src, size);
		unmap();
	}

	void read(void *dst, VkDeviceSize dataSize, VkDeviceSize offset = 0)
	{
		map(offset);
		memcpy(dst, (char *) data, dataSize);
		unmap();
	};

	void read(void *dst)
	{
		map();
		memcpy(dst, (char *) data, size);
		unmap();
	};

	void createView(ResourceTracker *pTracker, VkFormat format)
	{
		if (viewRes != nullptr)
		{
			return;
		}
		VkBufferViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
		viewInfo.buffer = buf;
		viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		viewInfo.offset = 0;
		viewInfo.range = VK_WHOLE_SIZE;
		ASSERT_VULKAN(vkCreateBufferView(gState.device.logical, &viewInfo, nullptr, &view));
		viewRes = new BufferView_R(view);
		pTracker->add(viewRes);
	}
	protected:
};

struct BufferDedicated : public Buffer
{
	BufferDedicated(ResourceTracker      *pTracker,
	                VkDeviceSize          deviceSize,
	                VkBufferUsageFlags    bufferUsageFlags,
	                VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		VkDeviceMemory mem;
		gState.memAlloc.createBufferDedicated(deviceSize, bufferUsageFlags, memoryPropertyFlags, &this->buf, &mem);
		this->size = deviceSize;
		this->res  = new BufferVK_R(this->buf, mem);
		pTracker->add(this->res);
	}
};

struct BufferVma : public Buffer
{
	BufferVma(ResourceTracker   *pTracker,
	          VkDeviceSize       deviceSize,
	          VkBufferUsageFlags bufferUsageFlags,
	          VmaMemoryUsage     memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
	{
		VmaAllocation alloc;
		gState.memAlloc.createBuffer(deviceSize, bufferUsageFlags, memoryUsage, &this->buf, &alloc);
		this->size = deviceSize;
		this->res  = new BufferVMA_R(this->buf, alloc);
		pTracker->add(this->res);
	}
};


}
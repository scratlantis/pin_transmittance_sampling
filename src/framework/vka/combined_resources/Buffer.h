#pragma once
#include "../resources/Resource.h"

namespace vka
{

class Buffer
{
  protected:
	MappableResource *res;
	BufferView_R     *viewRes = nullptr;

  public:
	VkBuffer          buf;
	VkBufferView      view;
	VkDeviceSize      size;

	void *data = nullptr;        // used for mapping

	void move(ResourceTracker* pNewTracker)
	{
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

	void createView(ResourceTracker *pTracker, VkFormat format, VkDeviceSize range, VkDeviceSize offset)
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
	void createView(ResourceTracker *pTracker, VkFormat format)
	{
		createView(pTracker, format, size, 0);
	}
	protected:
};

struct BufferDedicated : public Buffer
{
	BufferDedicated(ResourceTracker      *pTracker,
	                VkDeviceSize          deviceSize,
	                VkBufferUsageFlags    bufferUsageFlags,
	                VkMemoryPropertyFlags memoryPropertyFlags)
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
	BufferVma(ResourceTracker       *pTracker,
	                VkDeviceSize          deviceSize,
	                VkBufferUsageFlags    bufferUsageFlags,
	          VmaMemoryUsage     memoryUsage)
	{
		VmaAllocation alloc;
		gState.memAlloc.createBuffer(deviceSize, bufferUsageFlags, memoryUsage, &this->buf, &alloc);
		this->size = deviceSize;
		this->res  = new BufferVMA_R(this->buf, alloc);
		pTracker->add(this->res);
	}
};


}
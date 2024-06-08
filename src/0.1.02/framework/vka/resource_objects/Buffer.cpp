#include "Buffer.h"
#include <vka/state_objects/global_state.h>

namespace vka
{

void BufferVK_R::free()
{
	gState.memAlloc.destroyBuffer(handle, deviceMemory);
}

void* BufferVK_R::map(uint32_t offset, uint32_t size)
{
	void *data;
	gState.memAlloc.mapMemory(deviceMemory, offset, size, &data);
	return data;
}

void BufferVK_R::unmap()
{
	gState.memAlloc.unmapMemory(deviceMemory);
}

void BufferVMA_R::free()
{
	gState.memAlloc.destroyBuffer(handle, allocation);
}

void* BufferVMA_R::map(uint32_t offset, uint32_t size)
{
	void *data;
	gState.memAlloc.mapMemory(allocation, &data);
	data = (char *) data + offset;
	return data;
}
void BufferVMA_R::unmap()
{
	gState.memAlloc.unmapMemory(allocation);
}

void BufferView_R::free()
{
	vkDestroyBufferView(gState.device.logical, handle, nullptr);
}

void Buffer_I::create(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty)
{
	if (!hasMemoryOwnership)
	{
		this->type        = type;
		this->usage       = usage;
		this->memProperty = memProperty;
		this->size        = size;
		void          *heapPointer;
		VkDeviceMemory deviceMemory;
		VmaAllocation  allocation;
		switch (type)
		{
			case BufferType::VMA:
				gState.memAlloc.createBuffer(size, usage, memProperty.vma, &handle, &allocation);
				res = new BufferVMA_R(handle, allocation);
				break;
			case BufferType::VK:
				gState.memAlloc.createBufferDedicated(size, usage, memProperty.vk, &handle, &deviceMemory);
				res = new BufferVK_R(handle, deviceMemory);
				break;
			default:
				printVka("Invalid buffer type\n");
				DEBUG_BREAK;
				break;
		}
		hasMemoryOwnership = true;
	}
	else
	{
		vka::printVka("Cant recreate buffer with memory ownership\n");
		DEBUG_BREAK;
	}
}

Buffer_I Buffer_I::recreate(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, MemoryProperty memProperty, bool maintainData)
{
	Buffer_I bufferCopy = *this;
	if (maintainData)
	{
		//clang-format on
		if ((this->type == BufferType::VMA && this->memProperty.vma == VMA_MEMORY_USAGE_GPU_ONLY)
			|| (type == BufferType::VMA && memProperty.vma == VMA_MEMORY_USAGE_GPU_ONLY)
			|| (this->type == BufferType::VK && !(this->memProperty.vk & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
			|| (type == BufferType::VK && !(memProperty.vk & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)))
		//clang-format off
		{
			vka::printVka("Cant maintain data for GPU only buffer\n");
			DEBUG_BREAK;
			return bufferCopy;
		}
		else
		{
			Buffer_R    *oldRes      = res;
			VkDeviceSize minDataSize = std::min(this->size, size);
			free();
			create(type, size, usage, memProperty);
			void *oldData = oldRes->map(0, minDataSize);
			void *newData = map(0, minDataSize);
			std::memcpy(newData, oldData, minDataSize);
			unmap();
			oldRes->unmap();
		}
	}
	else
	{
		free();
		create(type, size, usage, memProperty);
	}
	return bufferCopy;
}

void Buffer_I::track(IResourcePool *pPool)
{
	if (!pPool)
	{
		vka::printVka("Null resource pool\n");
		DEBUG_BREAK;
		return;
	}
	if (viewRes)
	{
		viewRes->track(pPool);
	}
	if (res)
	{
		res->track(pPool);
	}
	Resource::track(pPool);
	hasMemoryOwnership = false;
}

void Buffer_I::free()
{
	if (isMapped)
	{
		vka::printVka("Cant free mapped buffer\n");
		DEBUG_BREAK;
	
	}
	if (hasMemoryOwnership)
	{
		vka::printVka("Cant free buffer with memory ownership\n");
		DEBUG_BREAK;
	}
	else
	{
		type           = BufferType::VMA;
		usage          = 0;
		memProperty.vk = 0;
		handle         = VK_NULL_HANDLE;
		viewHandle     = VK_NULL_HANDLE;
		pPool          = nullptr;
		size           = 0;
		res            = nullptr;
		viewRes        = nullptr;
	}
}
hash_t Buffer_I::hash() const
{
	return res->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}

void *Buffer_I::map(uint32_t offset, uint32_t size)
{
	isMapped = true;
	return res->map(offset, size);
}
void Buffer_I::unmap()
{
	isMapped = false;
	res->unmap();
}

}
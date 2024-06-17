#include "Buffer.h"
#include <vka/state_objects/global_state.h>

namespace vka
{

//void BufferVK_R::free()
//{
//	gState.memAlloc.destroyBuffer(handle, deviceMemory);
//}
//
//void* BufferVK_R::map(uint32_t offset, uint32_t size)
//{
//	void *data;
//	gState.memAlloc.mapMemory(deviceMemory, offset, size, &data);
//	return data;
//}
//
//void BufferVK_R::unmap() const
//{
//	gState.memAlloc.unmapMemory(deviceMemory);
//}

void BufferVMA_R::free()
{
	gState.memAlloc.destroyBuffer(handle, allocation);
}

void* BufferVMA_R::map(uint32_t offset, uint32_t size) const
{
	void *data;
	gState.memAlloc.mapMemory(allocation, &data);
	data = (char *) data + offset;
	return data;
}
void BufferVMA_R::unmap() const
{
	gState.memAlloc.unmapMemory(allocation);
}

void BufferView_R::free()
{
	vkDestroyBufferView(gState.device.logical, handle, nullptr);
}

void Buffer_I::createHandles()
{
	state.type = BufferType::VMA;
	VmaAllocation allocation;
	gState.memAlloc.createBuffer(state.size, state.usage, state.memProperty.vma, &handle, &allocation);
	res = new BufferVMA_R(handle, allocation);
	res->track(pPool);
}

void Buffer_I::detachChildResources()
{
	if (res)
	{
		res->track(gState.frame->stack);
		res = nullptr;
	}
	if (viewRes)
	{
		viewRes->track(gState.frame->stack);
		viewRes = nullptr;
	}
}

bool Buffer_I::isMappable() const
{
    //clang-format off
	return !(state.type == BufferType::NONE || (state.type == BufferType::VMA && state.memProperty.vma == VMA_MEMORY_USAGE_GPU_ONLY)
		|| (state.type == BufferType::VK && !(state.memProperty.vk & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)));
    //clang-format on
}

const Buffer_I Buffer_I::recreate()
{
	Buffer_I bufferCopy = *this;
	if (state.size == newState.size
		&& state.usage == newState.usage
		&& state.memProperty.vma == newState.memProperty.vma)
	{
		return bufferCopy;
	}
	state               = newState;
	detachChildResources();
	createHandles();
	return bufferCopy;
}


void Buffer_I::update()
{
	const Buffer_I oldBuffer = recreate();
	VkDeviceSize   minDataSize = std::min(oldBuffer.getSize(), getSize());
	void          *oldData     = oldBuffer.map(0, minDataSize);
	void          *newData     = this->map(0, minDataSize);
	std::memcpy(newData, oldData, minDataSize);
	unmap();
	oldBuffer.unmap();
}

void Buffer_I::writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const
{
	pBufferInfo->buffer = getHandle();
	pBufferInfo->offset = 0;
	pBufferInfo->range  = VK_WHOLE_SIZE;
	write.pBufferInfo   = pBufferInfo;
	pBufferInfo++;
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
}
hash_t Buffer_I::hash() const
{
	return res->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}

void *Buffer_I::map(uint32_t offset, uint32_t size) const
{
	return res->map(offset, size);
}
void Buffer_I::unmap() const
{
	res->unmap();
}

void Buffer_I::changeSize(VkDeviceSize size)
{
	newState.size = size;
}

void Buffer_I::addUsage(VkBufferUsageFlags usage)
{
	newState.usage |= usage;
}

void Buffer_I::changeUsage(VkBufferUsageFlags usage)
{
	newState.usage = usage;
}

void Buffer_I::changeMemoryType(VmaMemoryUsage memProperty)
{
	newState.memProperty.vma = memProperty;
}

VkDeviceSize Buffer_I::getSize() const
{
	return state.size;
}
VkBufferUsageFlags Buffer_I::getUsage() const
{
	return state.usage;
}

VmaMemoryUsage Buffer_I::getMemoryType() const
{
	return state.memProperty.vma;
}

const Buffer_R *Buffer_I::getMappable() const
{
	return res;
}

}
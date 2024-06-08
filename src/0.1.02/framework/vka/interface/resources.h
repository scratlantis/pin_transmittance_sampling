#pragma once
#include <vka/state_objects/global_state.h>
#include <vka/resource_objects/resource_common.h>

namespace vka
{

typedef Buffer_I    *VkaBuffer;
typedef Image_I     *VkaImage;
typedef CmdBuffer_I *VkaCommandBuffer;


struct VkaBufferCreateInfo
{
	BufferType         type;
	VkDeviceSize       size;
	VkBufferUsageFlags usage;
	MemoryProperty     memProperty;
};

void vkaReloadBuffer(VkaBuffer buffer, VkaBufferCreateInfo &ci, bool maintainData)
{
	if (buffer->type != ci.type || buffer->size != ci.size || buffer->usage != ci.usage ||
	    std::memcmp(&buffer->memProperty, &ci.memProperty, sizeof(MemoryProperty)) != 0)
	{
		buffer->garbageCollect();
		buffer->recreate(ci.type, ci.size, ci.usage, ci.memProperty, maintainData);
	}
}

void vkaGetBufferCI(VkaBuffer buffer, VkaBufferCreateInfo &ci)
{
	ci.type        = buffer->type;
	ci.size        = buffer->size;
	ci.usage       = buffer->usage;
	ci.memProperty = buffer->memProperty;
}

bool vkaAddBufferUsage(VkaBuffer buffer, VkBufferUsageFlags usage)
{
	if ((buffer->usage & usage) != usage)
	{
		VkaBufferCreateInfo ci;
		vkaGetBufferCI(buffer, ci);
		ci.usage |= usage;
		vkaReloadBuffer(buffer, ci, true);
		return true;
	}
	return false;
}
// void vkaCopyBuffer(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaBuffer dst, VkBufferCopy copy)
//{
//	vkaAddBufferUsage(src, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
//	vkaAddBufferUsage(dst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
//	// Todo: Implement command buffer
// }


}

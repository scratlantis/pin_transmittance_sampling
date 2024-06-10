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

VkaBufferCreateInfo vkaGetBufferCI(VkaBuffer buffer)
{
	VkaBufferCreateInfo ci;
	ci.type        = buffer->type;
	ci.size        = buffer->size;
	ci.usage       = buffer->usage;
	ci.memProperty = buffer->memProperty;
	return ci;

}

bool vkaAddBufferUsage(VkaBuffer buffer, VkBufferUsageFlags usage)
{
	if ((buffer->usage & usage) != usage)
	{
		VkaBufferCreateInfo ci = vkaGetBufferCI(buffer);
		ci.usage |= usage;
		vkaReloadBuffer(buffer, ci, true);
		return true;
	}
	return false;
}

void* vkaRemap(VkaBuffer buffer, uint32_t size)
{
	if (!buffer->isMappable() || buffer->size != size)
	{
		VkaBufferCreateInfo ci = vkaGetBufferCI(buffer);
		ci.memProperty.vma     = VMA_MEMORY_USAGE_CPU_ONLY;
		ci.size                = size;
		ci.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		ci.type = BufferType::VMA;
		vkaReloadBuffer(buffer, ci, false);
	}
	return buffer->map(0, size);
}

void vkaUnmap(VkaBuffer buffer)
{
	buffer->unmap();
}

void vkaRewrite(VkaBuffer buffer, void *data, uint32_t size)
{
	void *bufferData = vkaRemap(buffer, size);
	memcpy(bufferData, data, size);
	vkaUnmap(buffer);
}

VkaImage vkaGetSwapchainImage()
{
	if (gState.io.swapchainImage != nullptr)
	{
		delete gState.io.swapchainImage;
	}
	gState.io.swapchainImage = new SwapchainImage_I();
	return gState.io.swapchainImage;
}

VkaImage vkaCreateSwapchainAttachment(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags)
{
	VkImageCreateInfo ci = ImageCreateInfo_Default(usageFlags, gState.io.extent, format);
	VkaImage img         = new Image_I(ci, true);
	img->track(pPool);
	return img;
}

VkaBuffer vkaCreateBuffer(IResourcePool *pPool, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage)
{
	MemoryProperty memProperty;
	memProperty.vma  = memUsage;
	VkaBuffer buffer = new Buffer_I(BufferType::VMA, size, usage, memProperty);
	buffer->track(pPool);
	return buffer;
}

VkaBuffer vkaCreateBuffer(IResourcePool *pPool)
{
	VkaBuffer buffer = new Buffer_I();
	buffer->track(pPool);
	return buffer;
}

VkaBuffer vkaCreateBuffer(IResourcePool *pPool, VkBufferUsageFlags usage)
{
	VkaBuffer buffer = new Buffer_I();
	buffer->track(pPool);
	return buffer;
}

void vkaUpload(VkaCommandBuffer cmdBuf, VkaBuffer buffer)
{
	VKA_CHECK(buffer->type == BufferType::VMA); // Todo: Implement VK buffer upload
	MemoryProperty memProperty;
	memProperty.vma = VMA_MEMORY_USAGE_GPU_ONLY;
	buffer->garbageCollect();
	Buffer_I     oldBuffer = buffer->recreate(BufferType::VMA, buffer->size, buffer->usage, memProperty, false);
	VkBufferCopy copy      = {0,0,buffer->size};
	vkCmdCopyBuffer(cmdBuf->getHandle(), oldBuffer.getHandle(), buffer->getHandle(), 1, &copy);
}



// void vkaCopyBuffer(VkaCommandBuffer cmdBuf, VkaBuffer src, VkaBuffer dst, VkBufferCopy copy)
//{
//	vkaAddBufferUsage(src, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
//	vkaAddBufferUsage(dst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
//	// Todo: Implement command buffer
// }


}

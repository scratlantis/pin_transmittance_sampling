#include "MemAllocator.h"
#include "../global_state.h"

namespace vka
{
MemAllocator::MemAllocator()
{
}

void MemAllocator::init()
{
	ASSERT_TRUE(gState.initBits & STATE_INIT_DEVICE_BIT);
	ASSERT_TRUE(!(gState.initBits & STATE_INIT_MEMALLOC_BIT));
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice         = gState.device.physical;
	allocatorInfo.device                 = gState.device.logical;
	allocatorInfo.instance               = gState.device.instance;
	allocatorInfo.flags                  = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
	gState.initBits |= STATE_INIT_MEMALLOC_BIT;
}

void MemAllocator::destroy()
{
	ASSERT_TRUE(gState.initBits & STATE_INIT_MEMALLOC_BIT);
	vmaDestroyAllocator(vmaAllocator);
	gState.initBits &= ~STATE_INIT_MEMALLOC_BIT;
}

MemAllocator::~MemAllocator()
{
}

void MemAllocator::createImage(
    const VkImageCreateInfo *pImageCreateInfo,
    VmaAllocationCreateInfo *pVmaAllocationCreateInfo,
    VkImage                 *pImage,
    VmaAllocation           *pAllocation)
{
	ASSERT_VULKAN(vmaCreateImage(this->vmaAllocator, pImageCreateInfo, pVmaAllocationCreateInfo, pImage, pAllocation, nullptr));
}

void MemAllocator::destroyImage(VkImage &image, VmaAllocation &allocation)
{
	vmaDestroyImage(this->vmaAllocator, image, allocation);
}

void MemAllocator::mapMemory(VmaAllocation &allocation, void **ppData)
{
	vmaMapMemory(this->vmaAllocator, allocation, ppData);
}
void MemAllocator::unmapMemory(VmaAllocation &allocation)
{
	vmaUnmapMemory(this->vmaAllocator, allocation);
}

void MemAllocator::createBuffer(
    VkBufferCreateInfo      *pBufferCreateInfo,
    VmaAllocationCreateInfo *pVmaAllocationCreateInfo,
    VkBuffer                *pBuffer,
    VmaAllocation           *pAllocation,
    VmaAllocationInfo       *pAllocationInfo)
{
	vmaCreateBuffer(this->vmaAllocator, pBufferCreateInfo, pVmaAllocationCreateInfo, pBuffer, pAllocation, pAllocationInfo);
	ASSERT_TRUE((pAllocation != nullptr));
}

void MemAllocator::destroyBuffer(VkBuffer &buffer, VmaAllocation &allocation)
{
	vmaDestroyBuffer(this->vmaAllocator, buffer, allocation);
}
}        // namespace Vka
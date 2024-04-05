#pragma once
#include "../core/macros/macros.h"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vka
{
class MemAllocator
{
  public:
	MemAllocator();
	void init();
	void destroy();
	~MemAllocator();

	void createImage(const VkImageCreateInfo *pImageCreateInfo, VmaAllocationCreateInfo *pVmaAllocationCreateInfo, VkImage *pImage, VmaAllocation *pAllocation);
	void destroyImage(VkImage &image, VmaAllocation &allocation);
	void mapMemory(VmaAllocation &allocation, void **ppData);
	void unmapMemory(VmaAllocation &allocation);
	void createBuffer(VkBufferCreateInfo *pBufferCreateInfo, VmaAllocationCreateInfo *pVmaAllocationCreateInfo, VkBuffer *pBuffer, VmaAllocation *pAllocation, VmaAllocationInfo *pAllocationInfo);
	void destroyBuffer(VkBuffer &buffer, VmaAllocation &allocation);

	DELETE_COPY_CONSTRUCTORS(MemAllocator)
  private:
	VmaAllocator vmaAllocator;
};
}        // namespace vka
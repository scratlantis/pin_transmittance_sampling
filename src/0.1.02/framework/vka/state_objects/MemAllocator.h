#pragma once
#include <vka/core/common.h>
#include <vk_mem_alloc.h>

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
	void mapMemory(VkDeviceMemory &mem, uint32_t offset, uint32_t size, void **ppData);
	void unmapMemory(VkDeviceMemory &mem);
	void unmapMemory(VmaAllocation &allocation);
	void createBuffer(VkBufferCreateInfo *pBufferCreateInfo, VmaAllocationCreateInfo *pVmaAllocationCreateInfo, VkBuffer *pBuffer, VmaAllocation *pAllocation, VmaAllocationInfo *pAllocationInfo);
	void createBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags vkBufferUsageFlags, VmaMemoryUsage vmaMemoryUsageFlags, VkBuffer *buf, VmaAllocation *alloc);
	void createUnallocatedBuffer(VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags, VkBuffer *buf);
	void allocateBuffer(VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkMemoryRequirements memoryRequirements, VkDeviceMemory *mem);
	void createBufferDedicated(VkDeviceSize deviceSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer *buf, VkDeviceMemory *mem);
	void destroyBuffer(VkBuffer &buffer, VmaAllocation &allocation);

	void destroyBuffer(VkBuffer &buffer, VkDeviceMemory &deviceMemory);

	void destroyImage(VkImage &image, VkDeviceMemory &deviceMemory);

	DELETE_COPY_CONSTRUCTORS(MemAllocator)
  private:
	VmaAllocator vmaAllocator;
};
}        // namespace vka
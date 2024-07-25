#pragma once
#include "types.h"
#include <vka/core/core_state/IResourcePool.h>

namespace vka
{
Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags);
Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VkDeviceSize size);

void *writePtr(Buffer buf, uint32_t size);
void  write(Buffer buf, uint32_t size, const void *data);
void  fill(Buffer buf, uint32_t size, const void *data, uint32_t count);

}        // namespace vka
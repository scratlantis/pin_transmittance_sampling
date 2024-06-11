#pragma once
#include <vka/state_objects/global_state.h>
#include <vka/resource_objects/resource_common.h>
#include "types.h"

namespace vka
{
	// Create
	VkaBuffer vkaCreateBuffer(IResourcePool* pPool, VkBufferUsageFlags usageFlags = 0)
	{
	    return new Buffer_I(pPool, usageFlags);
	}

	// Read/Write
	void *vkaMapStageing(VkaBuffer buffer, uint32_t size)
	{
		buffer->addUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		buffer->changeSize(size);
		buffer->changeMemoryType(VMA_MEMORY_USAGE_CPU_ONLY);
	    buffer->recreate();
		return buffer->map(0, size);
	}
	void vkaUnmap(VkaBuffer buffer)
	{
		buffer->unmap();
	}
	void vkaWriteStaging(VkaBuffer buffer, void* data, uint32_t size)
	{
	    void* mapping = vkaMapStageing(buffer, size);
		memcpy(mapping, data, size);
		vkaUnmap(buffer);
	}

	

}
#include "buffer_functionality.h"
using namespace vka;

// Create
VkaBuffer vkaCreateBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags)
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
void vkaWriteStaging(VkaBuffer buffer, void *data, uint32_t size)
{
	void *mapping = vkaMapStageing(buffer, size);
	memcpy(mapping, data, size);
	vkaUnmap(buffer);
}

void vkaFillStaging(VkaBuffer buffer, void *data, uint32_t size, uint32_t count)
{
	void *mapping = vkaMapStageing(buffer, size*count);
	for (size_t i = 0; i < count; i++)
	{
		memcpy((char *) mapping + i * size, data, size);
	}
	vkaUnmap(buffer);
}

void vkaRead(VkaBuffer buffer, void *dst)
{
	void *mapping = buffer->map(0, buffer->getSize());
	memcpy(dst, mapping, buffer->getSize());
	buffer->unmap();
}

#pragma once
#include <vka/state_objects/global_state.h>
#include <vka/resource_objects/resource_common.h>
#include "types.h"

// Create
VkaBuffer vkaCreateBuffer(vka::IResourcePool *pPool, VkBufferUsageFlags usageFlags = 0);

// Read/Write
void *vkaMapStageing(VkaBuffer buffer, uint32_t size);
void  vkaUnmap(VkaBuffer buffer);
void  vkaWriteStaging(VkaBuffer buffer, const void *data, uint32_t size);
void  vkaFillStaging(VkaBuffer buffer, const void *data, uint32_t size, uint32_t count);
void  vkaRead(VkaBuffer buffer, void *dst);

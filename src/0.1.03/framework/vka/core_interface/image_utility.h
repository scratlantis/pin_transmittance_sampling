#pragma once
#include "types.h"
#include <vka/core/core_state/IResourcePool.h>
namespace vka
{
Image vkaCreateImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent2D extent);
Image vkaCreateImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
}        // namespace vka
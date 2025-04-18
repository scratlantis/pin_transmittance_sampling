#pragma once
#include <vka/state_objects/global_state.h>
#include <vka/resource_objects/resource_common.h>
#include "types.h"

using namespace vka;
VkaImage vkaCreateSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);
VkaImage vkaGetSwapchainImage();

VkaImage vkaCreateImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent2D extent);

VkaImage vkaCreateImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

#pragma once
#include <vka/state_objects/global_state.h>
#include <vka/resource_objects/resource_common.h>
#include "types.h"

namespace vka
{
	VkaImage vkaCreateSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED)
	{
		VkImageCreateInfo ci  = ImageCreateInfo_Default(usageFlags, gState.io.extent, format);
	    ci.initialLayout     = initialLayout;
	    VkaImage          img = new Image_I(gState.io.swapchainAttachmentPool, ci, true);
	    img->createHandles();
	    return img;
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
}        // namespace vka
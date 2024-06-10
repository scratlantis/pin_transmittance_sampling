#pragma once
#include <vka/state_objects/global_state.h>
#include <vka/resource_objects/resource_common.h>
#include "types.h"

namespace vka
{
	VkaImage vkaCreateSwapchainAttachment(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags)
	{
		VkImageCreateInfo ci  = ImageCreateInfo_Default(usageFlags, gState.io.extent, format);
		VkaImage          img = new Image_I(pPool, ci, true);
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
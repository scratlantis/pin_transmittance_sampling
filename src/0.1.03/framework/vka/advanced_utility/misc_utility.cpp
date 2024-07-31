#include "misc_utility.h"
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/stateless/vk_types/default_values.h>
#include <vka/globals.h>

namespace vka
{
void swapBuffers(std::vector<CmdBuffer> cmdBufs)
{
	SubmitSynchronizationInfo syncInfo = gState.acquireNextSwapchainImage();
	submit(cmdBufs, gState.device.universalQueues[0], syncInfo);
	vkDeviceWaitIdle(gState.device.logical);
	gState.presentFrame();
	gState.nextFrame();
}

vka::VkRect2D_OP getScissorRect(float x, float y, float width, float height)
{
	return vka::VkRect2D_OP{{{0, 0}, gState.io.extent}} * Rect2D<float>{x, y, std::min(width, 1.0f - x), std::min(height, 1.0f - y)};
}


Image createSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout)
{
	VkImageCreateInfo ci = ImageCreateInfo_Default(usageFlags, gState.io.extent, format);
	ci.initialLayout     = initialLayout;
	Image img         = new Image_R(gState.swapchainAttachmentPool, ci, true);
	img->createHandles();
	return img;
}

Image getSwapchainImage()
{
	if (gState.swapchainImage != nullptr)
	{
		delete gState.swapchainImage;
	}
	gState.swapchainImage = new SwapchainImage_R();
	return gState.swapchainImage;
}

}		// namespace vka
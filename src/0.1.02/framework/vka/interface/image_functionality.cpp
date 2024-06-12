#include "image_functionality.h"

using namespace vka;
VkaImage vkaCreateSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout)
{
	VkImageCreateInfo ci = ImageCreateInfo_Default(usageFlags, gState.io.extent, format);
	ci.initialLayout     = initialLayout;
	VkaImage img         = new Image_I(gState.io.swapchainAttachmentPool, ci, true);
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
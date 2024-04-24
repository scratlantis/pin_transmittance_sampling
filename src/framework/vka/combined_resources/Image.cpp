#include "Image.h"
#include "../initializers/misc.h"
namespace vka
{
SwapchainImage::SwapchainImage()
{
}
SwapchainImage::~SwapchainImage()
{
}
SwapchainImage::SwapchainImage(ResourceTracker *pTracker, VkImage swapchainImage, VkFormat swapchainFormat, VkExtent2D swapchainExtent)
{
	layout                            = VK_IMAGE_LAYOUT_UNDEFINED;
	format                            = swapchainFormat;
	extent                            = getExtent3D(swapchainExtent);
	mipLevels                         = 1;
	usage                             = 0;
	img                               = swapchainImage;
	VkImageViewCreateInfo imageViewCI = ImageViewCreateInfo_Swapchain(swapchainImage, format);
	ASSERT_VULKAN(vkCreateImageView(gState.device.logical, &imageViewCI, nullptr, &view));
	viewRes = new ImageView_R(view);
	pTracker->add(viewRes);
}
}        // namespace vka
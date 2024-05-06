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
	VkImageViewCreateInfo imageViewCI = ImageViewCreateInfo_Default(swapchainImage, format);
	ASSERT_VULKAN(vkCreateImageView(gState.device.logical, &imageViewCI, nullptr, &view));
	viewRes = new ImageView_R(view);
	pTracker->add(viewRes);
}

FramebufferImage::FramebufferImage(ResourceTracker *pTracker, VkImageUsageFlags usage, VkFormat format, VkExtent2D extent)
{
	VkImageCreateInfo imgCI = ImageCreateInfo_Default(usage, extent, format);
	create(pTracker, imgCI);
}

void FramebufferImage::create(ResourceTracker *pTracker, const VkImageCreateInfo &imgCI)
{
	VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
	vmaAllocationCreateInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;
	VmaAllocation alloc;
	gState.memAlloc.createImage(&imgCI, &vmaAllocationCreateInfo, &img, &alloc);
	layout    = VK_IMAGE_LAYOUT_UNDEFINED;
	format    = imgCI.format;
	extent    = imgCI.extent;
	mipLevels = imgCI.mipLevels;
	usage     = imgCI.usage;
	res       = new ImageVMA_R(img, alloc);
	pTracker->add(res);
	createImageView(pTracker, ImageViewCreateInfo_Default(img, format));
}

void FramebufferImage::recreate(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker)
{
	destroy(pGarbageTracker);
	VkExtent2D newExtent = {extent.width, extent.height};
	VkImageCreateInfo imgCI     = ImageCreateInfo_Default(usage, newExtent, format);
	create(pTracker, imgCI);
}

void FramebufferImage::update(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker, VkExtent2D newExtent)
{
	if (this->extent.width == newExtent.width && this->extent.height == newExtent.height)
	{
		return;
	}
	extent.width = newExtent.width;
	extent.height = newExtent.height;
	recreate(pTracker, pGarbageTracker);

}
void FramebufferImage::update(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker, VkImageUsageFlags newUsage)
{
	if (this->usage == newUsage)
	{
		return;
	}
	usage = newUsage;
	recreate(pTracker, pGarbageTracker);
}

void FramebufferImage::update(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker, VkFormat newFormat)
{
	if (this->format == newFormat)
	{
		return;
	}
	format = newFormat;
	recreate(pTracker, pGarbageTracker);
}

void FramebufferImage::destroy(ResourceTracker *pGarbageTracker)
{
	ASSERT_TRUE(res != nullptr && viewRes != nullptr);
	res->move(pGarbageTracker);
	viewRes->move(pGarbageTracker);
	res = nullptr;
	viewRes = nullptr;
}


FramebufferImage::~FramebufferImage()
{
}

FramebufferImage::FramebufferImage()
{
}
ImageVma::ImageVma()
{
}
void Image::createImageView(ResourceTracker *pTracker)
{
	VkImageViewCreateInfo viewCI = ImageViewCreateInfo_Default(img, format);
	if (viewRes != nullptr)
	{
		return;
	}
	ASSERT_VULKAN(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &view));
	viewRes = new ImageView_R(view);
	pTracker->add(viewRes);
}

}        // namespace vka
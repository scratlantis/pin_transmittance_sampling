#include "Image.h"
#include <vka/state_objects/global_state.h>

namespace vka
{
void ImageVMA_R::free()
{
	gState.memAlloc.destroyImage(handle, allocation);
}
void ImageView_R::free()
{
	vkDestroyImageView(gState.device.logical, handle, nullptr);
}
void Image_I::create(const VkImageCreateInfo &imgCI, bool createView, VmaMemoryUsage memUsage)
{
	VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
	vmaAllocationCreateInfo.usage                   = memUsage;
	VmaAllocation alloc;
	gState.memAlloc.createImage(&imgCI, &vmaAllocationCreateInfo, &handle, &alloc);

	layout    = VK_IMAGE_LAYOUT_UNDEFINED;
	format    = imgCI.format;
	extent    = imgCI.extent;
	mipLevels = imgCI.mipLevels;
	usage     = imgCI.usage;
	res       = new ImageVMA_R(handle, alloc);
	if (createView)
	{
		VkImageViewCreateInfo viewCI = ImageViewCreateInfo_Default(handle, format);
		VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &viewHandle));
		viewRes = new ImageView_R(viewHandle);
	}
	hasMemoryOwnership = true;
}
Image_I Image_I::recreate(const VkImageCreateInfo &imgCI, bool createView, VmaMemoryUsage memUsage)
{
	Image_I imgCopy = *this;
	free();
	create(imgCI, createView, memUsage);
	return imgCopy;
}

void Image_I::track(IResourcePool *pPool)
{
	if (!pPool)
	{
		vka::printVka("Null resource pool\n");
		DEBUG_BREAK;
		return;
	}
	if (viewRes)
	{
		viewRes->track(pPool);
	}
	if (res)
	{
		res->track(pPool);
	}
	Resource::track(pPool);
	hasMemoryOwnership = false;
}

void Image_I::free()
{
	if (!hasMemoryOwnership)
	{
		res        = nullptr;
		viewRes    = nullptr;
		pPool      = nullptr;
		handle     = VK_NULL_HANDLE;
		viewHandle = VK_NULL_HANDLE;
	}
	else
	{
		vka::printVka("Cant free buffer with memory ownership\n");
		DEBUG_BREAK;
	}
}
hash_t Image_I::hash() const
{
	return res->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}
VkImage SwapchainImage_I::getHandle() const
{
	return gState.io.images[gState.frame->frameIndex];
}
VkImageView SwapchainImage_I::getViewHandle() const
{
	return gState.io.imageViews[gState.frame->frameIndex];
}
VkFormat SwapchainImage_I::getFormat() const
{
	return gState.io.format;
}
VkExtent3D SwapchainImage_I::getExtent() const
{
	return {gState.io.extent.width, gState.io.extent.height, 1};
}
VkImageUsageFlags SwapchainImage_I::getUsage() const
{
	return gState.io.imageUsage;
}
VkImageLayout SwapchainImage_I::getLayout() const
{
	return gState.io.imageLayouts[gState.frame->frameIndex];
}
void SwapchainImage_I::setLayout(VkImageLayout layout)
{
	gState.io.imageLayouts[gState.frame->frameIndex] = layout;
}
hash_t SwapchainImage_I::hash() const
{
	return (hash_t) gState.io.images[gState.frame->frameIndex] << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}
}        // namespace vka
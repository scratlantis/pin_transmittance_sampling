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

void Image_I::createHandles()
{
	layout    = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo vmaAllocationCreateInfo{};
	vmaAllocationCreateInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;
	VmaAllocation alloc;
	gState.memAlloc.createImage(&ci, &vmaAllocationCreateInfo, &handle, &alloc);
	res       = new ImageVMA_R(handle, alloc);
	res->track(pPool);
	if (createView)
	{
		VkImageViewCreateInfo viewCI = ImageViewCreateInfo_Default(handle, format);
		VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &viewHandle));
		viewRes = new ImageView_R(viewHandle);
		viewRes->track(pPool);
	}
}


Image_I Image_I::recreate()
{
	Image_I imgCopy = *this;
	// clang-format off
	if (ci.format == format &&
		ci.extent.depth == extent.depth && ci.extent.height == extent.height && ci.extent.width == extent.width
		&& ci.mipLevels == mipLevels
		&& ci.usage == usage)
	// clang-format on
	{
		return imgCopy;
	}
	ci.format    = format;
	ci.extent    = extent;
	ci.mipLevels = mipLevels;
	ci.usage     = usage;
	detachChildResources();
	createHandles();
	return imgCopy;
}

void Image_I::detachChildResources()
{
	if (res)
	{
		res->track(gState.frame->stack);
	}
	if (viewRes)
	{
		viewRes->track(gState.frame->stack);
	}
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
}

void Image_I::free()
{
}
hash_t Image_I::hash() const
{
	if (res)
		return res->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
	return 0;
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

void Image_I::writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const
{
	pImageInfos->imageView   = viewHandle;
	pImageInfos->imageLayout = layout;
	write.pImageInfo    = pImageInfos;
	pImageInfos++;
}
}        // namespace vka
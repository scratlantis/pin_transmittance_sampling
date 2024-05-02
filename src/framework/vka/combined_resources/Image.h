#pragma once
#include "../resources/Resource.h"
namespace vka
{
class Image
{
  protected:
	NonUniqueResource *res     = nullptr;
	ImageView_R      *viewRes = nullptr;

  public:
	VkImage     img;
	VkImageView view;

	// Set on creation
	VkFormat   format;
	VkExtent3D extent;
	uint32_t   mipLevels;
	VkImageUsageFlags usage;

	// Set on creation & updated on layout transform or renderpass (for framebuffer images)
	VkImageLayout layout;
	Image() = default;

	VkDeviceSize getSize()
	{
		VkDeviceSize size = extent.width * extent.height * extent.depth;
		size *= mipLevels;
		size *= cVkFormatTable.at(format).size;
		return size;
	}
	void move(ResourceTracker *pNewTracker)
	{
		ASSERT_TRUE(res != nullptr);
		res->move(pNewTracker);
		if (viewRes != nullptr)
		{
			viewRes->move(pNewTracker);
		}
	}

	void moveView(ResourceTracker *pNewTracker)
	{
		if (viewRes != nullptr)
		{
			viewRes->move(pNewTracker);
		}
	}

	void createImageView(ResourceTracker *pTracker, const VkImageViewCreateInfo &viewCI)
	{
		if (viewRes != nullptr)
		{
			return;
		}
		ASSERT_VULKAN(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &view));
		viewRes = new ImageView_R(view);
		pTracker->add(viewRes);
	}

};

// Memory allocated by VMA
class ImageVma : public Image
{
  public:
	ImageVma();
	ImageVma(ResourceTracker *pTracker, const VkImageCreateInfo &imgCI, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY)
	  {
		  VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
		  vmaAllocationCreateInfo.usage                   = memUsage;
		  VmaAllocation alloc;
		  gState.memAlloc.createImage(&imgCI, &vmaAllocationCreateInfo, &img, &alloc);

		  layout      = VK_IMAGE_LAYOUT_UNDEFINED;
		  format      = imgCI.format;
		  extent      = imgCI.extent;
		  mipLevels   = imgCI.mipLevels;
		  usage		  = imgCI.usage;
		  res = new ImageVMA_R(img, alloc);
		  pTracker->add(res);
	}
};

class SwapchainImage : public ImageVma
{
  public:
	SwapchainImage();
	~SwapchainImage();
	SwapchainImage(ResourceTracker *pTracker, VkImage swapchainImage, VkFormat swapchainFormat, VkExtent2D swapchainExtent);
};

// provide special functionality for framebuffer images
class FramebufferImage : public ImageVma
{
	
  protected:
	void create(ResourceTracker *pTracker, const VkImageCreateInfo &imgCI);
	void recreate(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker);
	void destroy(ResourceTracker *pGarbageTracker);
  public:
	FramebufferImage(ResourceTracker *pTracker, VkImageUsageFlags usage, VkFormat format, VkExtent2D extent);
	void update(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker, VkExtent2D newExtent);
	void update(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker, VkImageUsageFlags newUsage);
	void update(ResourceTracker *pTracker, ResourceTracker *pGarbageTracker, VkFormat newFormat);
	~FramebufferImage();
	FramebufferImage();
};

}        // namespace vka
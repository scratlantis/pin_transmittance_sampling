#pragma once
#include "../resources/Resource.h"
namespace vka
{
class Image
{
  protected:
	NonUniqueResource *res;
	ImageView_R      *viewRes = nullptr;

  public:
	VkImage     img;
	VkImageView view;

	// Set on creation
	VkFormat   format;
	VkExtent3D extent;
	uint32_t   mipLevels;

	// Set on creation & updated on layout transform or renderpass (for framebuffer images)
	VkImageLayout layout;

	void move(ResourceTracker *pNewTracker)
	{
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
};

// Resource Owned by the swapchain
class SwapchainImage : public Image
{

};

// Dedicated allocation
class ImageDedicated : public Image
{

};

// Memory allocated by VMA
class ImageVma : public Image
{
  public:
	ImageVma(ResourceTracker *pTracker, const VkImageCreateInfo imgCI, VmaMemoryUsage memUsage)
	  {
		  VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
		  vmaAllocationCreateInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;
		  VmaAllocation alloc;
		  gState.memAlloc.createImage(&imgCI, &vmaAllocationCreateInfo, &img, &alloc);

		  layout      = VK_IMAGE_LAYOUT_UNDEFINED;
		  format      = imgCI.format;
		  extent      = imgCI.extent;
		  mipLevels   = imgCI.mipLevels;

		  res = new ImageVMA_R(img, alloc);
		  pTracker->add(res);
	}
};


// provide special functionality for framebuffer images
class FramebufferImage : public ImageVma
{


  public:
	FramebufferImage();
	~FramebufferImage();
};

}        // namespace vka
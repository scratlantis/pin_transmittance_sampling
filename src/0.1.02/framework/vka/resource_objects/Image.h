#pragma once
#include "Resource.h"
#include <vma/vk_mem_alloc.h>
namespace vka
{

class ImageVMA_R : public Resource_T<VkImage>
{
  public:
	ImageVMA_R(VkImage handle, VmaAllocation allocation) :
		Resource_T<VkImage>(handle)
	{
		this->allocation = allocation;
	}
	void free() override;
  private:
	VmaAllocation allocation;
};

class ImageView_R : public Resource_T<VkImageView>
{
  public:
	ImageView_R(VkImageView handle) :
	    Resource_T<VkImageView>(handle){}
	void free() override;
};

class Image_I : Resource_T<VkImage>
{
  protected:
	Resource *res     = nullptr;
	Resource *viewRes = nullptr;
	bool hasMemoryOwnership = false;
  public:
	VkImageView viewHandle = VK_NULL_HANDLE;

	// State
	VkFormat          format;
	VkExtent3D        extent;
	uint32_t          mipLevels;
	VkImageUsageFlags usage;
	VkImageLayout     layout;

	Image_I() : Resource_T<VkImage>(VK_NULL_HANDLE)
	{
		format    = VK_FORMAT_UNDEFINED;
		extent    = {0, 0, 0};
		mipLevels = 0;
		usage     = 0;
		layout    = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	Image_I(const VkImageCreateInfo &imgCI, bool createView, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY):
		Resource_T<VkImage>(VK_NULL_HANDLE)
	{
		create(imgCI, createView, memUsage);
	}
	~Image_I()
	{
		free();
	}
  private:
	Image_I &operator=(const Image_I &rhs)
	{
		if (this == &rhs)
		{
			return *this;
		}
		// No ownership, no tracking
		res                = nullptr;
		viewRes            = nullptr;
		hasMemoryOwnership = false;
		pPool              = nullptr;

		handle     = rhs.handle;
		viewHandle = rhs.viewHandle;

		format    = rhs.format;
		extent    = rhs.extent;
		mipLevels = rhs.mipLevels;
		usage     = rhs.usage;
		layout    = rhs.layout;
		return *this;
	}
  public:
	void create(const VkImageCreateInfo &imgCI, bool createView, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY);
	Image_I recreate(const VkImageCreateInfo &imgCI, bool createView, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY);
	void track(IResourcePool *pPool) override;
	void   free() override;
	hash_t hash() const override;
};


} // namespace vka
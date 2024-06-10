#pragma once
#include "Resource.h"
#include <vma/vk_mem_alloc.h>
#include "IDescriptor.h"
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


class Image_I : public Resource_T<VkImage>, public IDescriptor
{
  protected:
	Resource *res     = nullptr;
	Resource *viewRes = nullptr;
	bool createView;
	Image_I() = default;
  private:
	VkImageView viewHandle = VK_NULL_HANDLE;

	// Current configuration
	VkImageCreateInfo ci;

	// Overrides
	VkFormat          format;
	VkExtent3D        extent;
	uint32_t          mipLevels;
	VkImageUsageFlags usage;


	VkImageLayout     layout;

  public:

	VkDeviceSize getMemorySize() const
	{
		VkDeviceSize size = extent.width * extent.height * extent.depth;
		size *= mipLevels;
		size *= cVkFormatTable.at(format).size;
		return size;
	}

	  virtual VkImageView getViewHandle() const
	  {
		  return viewHandle;
	  }
	  virtual VkFormat getFormat() const
	  {
		  return ci.format;
	  }
	  virtual VkExtent3D getExtent() const
	  {
		  return ci.extent;
	  }
	  virtual uint32_t getMipLevels() const
	  {
		  return ci.mipLevels;
	  }
	  virtual VkImageUsageFlags getUsage() const
	  {
		  return ci.usage;
	  }
	  virtual VkImageLayout getLayout() const
	  {
		  return layout;
	  }
	  virtual void setLayout(VkImageLayout layout)
	  {
		  this->layout = layout;
	  }
	  Image_I(IResourcePool *pPool, VkImageCreateInfo imgCI, bool createView) :
	      Resource_T<VkImage>(VK_NULL_HANDLE)
	{
		ci = imgCI;
		format    = ci.format;
		extent    = ci.extent;
		mipLevels = ci.mipLevels;
		usage     = ci.usage;
		layout    = ci.initialLayout;
		this->createView = createView;
		track(pPool);
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
	virtual void changeFormat(VkFormat format)
	{
		this->format = format;
	}
	virtual void changeExtent(VkExtent3D extent)
	{
		this->extent = extent;
	}
	virtual void changeMipLevels(uint32_t mipLevels)
	{
		this->mipLevels = mipLevels;
	}
	virtual void changeUsage(VkImageUsageFlags usage)
	{
		this->usage = usage;
	}


	virtual void createHandles();
	virtual Image_I recreate();
	virtual void       detachChildResources();
	virtual void     track(IResourcePool *pPool) override;
	virtual void     free() override;
	virtual hash_t    hash() const override;

	void writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const override;
};

class SwapchainImage_I : public Image_I
{
  public:
	VkImage getHandle() const override;
	VkImageView getViewHandle() const override;
	VkFormat    getFormat() const override;
	VkExtent3D  getExtent() const override;
	uint32_t    getMipLevels() const override
	{
		return 1;
	}
	VkImageUsageFlags getUsage() const override;
	VkImageLayout     getLayout() const override;
	void              setLayout(VkImageLayout layout) override;


	void changeFormat(VkFormat format) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}
	void changeExtent(VkExtent3D extent) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}
	void changeMipLevels(uint32_t mipLevels) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}
	void changeUsage(VkImageUsageFlags usage) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}

  public:
	void createHandles() override
	{
		printVka("Attempted to create handles for swapchain image");
		DEBUG_BREAK;
	};
	Image_I recreate() override
	{
		printVka("Attempted to recreate swapchain image");
		DEBUG_BREAK;
		return *this;
	};
	void detachChildResources() override {};
	void track(IResourcePool *pPool) override
	{
		printVka("Attempted to track swapchain image");
		DEBUG_BREAK;
	};
	void free() override
	{
		printVka("Attempted to free swapchain image");
		DEBUG_BREAK;
	};
	hash_t hash() const override;
};


} // namespace vka
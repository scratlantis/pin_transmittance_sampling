#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>
namespace vka
{

class ImageVMA_R : public Resource
{
  public:
	ImageVMA_R(VkImage handle, VmaAllocation allocation)
	{
		this->handle     = handle;
		this->allocation = allocation;
	}
	hash_t hash() const
	{
		return (hash_t) this->handle;
	}
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<ImageVMA_R const &>(other);
			return this->handle == other_.handle;
		}
	};

  protected:
	void free()
	{
		gState.memAlloc.destroyImage(handle, allocation);
	}

  private:
	VkImage      handle;
	VmaAllocation allocation;
};

class ImageView_R : public Resource
{
  public:
	ImageView_R(VkImageView handle)
	{
		this->handle     = handle;
	}
	hash_t hash() const
	{
		return (hash_t) this->handle;
	}
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<ImageView_R const &>(other);
			return this->handle == other_.handle;
		}
	};

  protected:
	void free()
	{
		vkDestroyImageView(gState.device.logical, handle, nullptr);
	}

  private:
	VkImageView       handle;
};

// Maybe Support later

//class ImageVK_R : public Resource
//{
//  public:
//	ImageVK_R(VkImage handle, VkDeviceMemory deviceMemory)
//	{
//		this->handle     = handle;
//		this->deviceMemory = deviceMemory;
//	}
//	hash_t hash() const
//	{
//		return (hash_t) this->handle;
//	}
//	bool _equals(Resource const &other) const
//	{
//		if (typeid(*this) != typeid(other))
//			return false;
//		else
//		{
//			auto &other_ = static_cast<ImageVK_R const &>(other);
//			return this->handle == other_.handle;
//		}
//	};
//
//  protected:
//	void free()
//	{
//		gState.memAlloc.destroyImage(handle, deviceMemory);
//	}
//
//  private:
//	VkImage       handle;
//	VkDeviceMemory deviceMemory;
//};

class Image_I : Resource
{
  protected:
	Resource *res     = nullptr;
	Resource *viewRes = nullptr;

  public:
	VkImage     img  = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;

	// State
	VkFormat          format;
	VkExtent3D        extent;
	uint32_t          mipLevels;
	VkImageUsageFlags usage;
	VkImageLayout     layout;

	bool hasMemoryOwnership = false;

	Image_I(const VkImageCreateInfo &imgCI, bool createView, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY)
	{
		VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
		vmaAllocationCreateInfo.usage                   = memUsage;
		VmaAllocation alloc;
		gState.memAlloc.createImage(&imgCI, &vmaAllocationCreateInfo, &img, &alloc);

		layout    = VK_IMAGE_LAYOUT_UNDEFINED;
		format    = imgCI.format;
		extent    = imgCI.extent;
		mipLevels = imgCI.mipLevels;
		usage     = imgCI.usage;
		res       = new ImageVMA_R(img, alloc);
		if (createView)
		{
			VkImageViewCreateInfo viewCI = ImageViewCreateInfo_Default(img, format);
			VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &view));
			viewRes = new ImageView_R(view);
		}
		hasMemoryOwnership = true;
	}
	~Image_I()
	{
		free();
	}

	Image_I& operator=(const Image_I& rhs)
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

		img = rhs.img;
		view = rhs.view;

		format = rhs.format;
		extent = rhs.extent;
		mipLevels = rhs.mipLevels;
		usage = rhs.usage;
		layout = rhs.layout;
		return *this;
	}
	void track(ResourcePool *pPool) override
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
	void garbageCollect() override
	{
		if (viewRes)
		{
			viewRes->garbageCollect();
		}
		if (res)
		{
			res->garbageCollect();
			track(&gState.frame->stack);
		}
	}
	void free() override
	{
		if (!hasMemoryOwnership)
		{
			res     = nullptr;
			viewRes = nullptr;
			pPool   = nullptr;
			img  = VK_NULL_HANDLE;
			view = VK_NULL_HANDLE;
		}
		else
		{
			vka::printVka("Cant free buffer with memory ownership\n");
			DEBUG_BREAK;
		}
	}
	hash_t hash() const
	{
		return res->hash() + VKA_RESOURCE_META_DATA_HASH_OFFSET;
	}
	bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		else
		{
			auto &other_ = static_cast<Image_I const &>(other);
			return *res == *other_.res;
		}
	};

	void recreate(const VkImageCreateInfo &imgCI, bool createView, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY)
	{
		free();
		VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
		vmaAllocationCreateInfo.usage                   = memUsage;
		VmaAllocation alloc;
		gState.memAlloc.createImage(&imgCI, &vmaAllocationCreateInfo, &img, &alloc);

		layout    = VK_IMAGE_LAYOUT_UNDEFINED;
		format    = imgCI.format;
		extent    = imgCI.extent;
		mipLevels = imgCI.mipLevels;
		usage     = imgCI.usage;
		res       = new ImageVMA_R(img, alloc);
		if (createView)
		{
			VkImageViewCreateInfo viewCI = ImageViewCreateInfo_Default(img, format);
			VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &view));
			viewRes = new ImageView_R(view);
		}
		hasMemoryOwnership = true;
	}

};


} // namespace vka
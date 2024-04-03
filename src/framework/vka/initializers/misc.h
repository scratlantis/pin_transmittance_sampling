#pragma once
#include <vulkan/vulkan.h>
namespace vka
{
struct ImageViewCreateInfo_Swapchain : public VkImageViewCreateInfo
{
	ImageViewCreateInfo_Swapchain(const VkImage &vkImage, const VkFormat &vkFormat);
};

ImageViewCreateInfo_Swapchain::ImageViewCreateInfo_Swapchain(const VkImage &vkImage, const VkFormat &vkFormat) :
    VkImageViewCreateInfo()
{
	sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	subresourceRange.baseMipLevel   = 0;
	subresourceRange.levelCount     = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount     = 1;
	image                           = vkImage;
	format                          = vkFormat;
	subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
}

}        // namespace vka
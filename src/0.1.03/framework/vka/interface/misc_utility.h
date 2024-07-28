#pragma once
#include <vka/core_interface/types.h>
namespace vka
{
void             swapBuffers(std::vector<CmdBuffer> cmdBufs);
vka::VkRect2D_OP getScissorRect(float x, float y, float width, float height);
Image            createSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout);
Image            vkaGetSwapchainImage();
}		// namespace vka
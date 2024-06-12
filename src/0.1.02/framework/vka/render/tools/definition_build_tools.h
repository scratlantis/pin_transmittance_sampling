#pragma once
#include <vka/resource_objects/resource_common.h>

namespace vka
{
inline ZERO_PAD(VkSubpassDependency)
initialSubpassDependency()
{
	ZERO_PAD(VkSubpassDependency)
	dependency{};
	dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass      = 0;
	dependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependency;
}
inline ZERO_PAD(VkSubpassDependency)
    finalSubpassDependency()
{
	ZERO_PAD(VkSubpassDependency)
	dependency{};
	dependency.srcSubpass      = 0;
	dependency.dstSubpass      = VK_SUBPASS_EXTERNAL;
	dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependency;
}

inline void setDefaults(RenderPassDefinition &def, uint32_t maxSubpassCount, uint32_t maxAttachmentCount)
{
	def = {};
	def.attachmentDescriptions.reserve(maxAttachmentCount);
	def.attachmentReferences.reserve(maxAttachmentCount * maxSubpassCount);
	SubpassDescription_OP subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	def.subpassDescriptions              = {subpassDescription};
	def.subpassDependencies              = {initialSubpassDependency(), finalSubpassDependency()};
}

struct AttachmentLayoutDescription
{
	VkImageLayout in;
	VkImageLayout internal;
	VkImageLayout out;
};


inline void addAttachment(RenderPassDefinition &def, AttachmentLayoutDescription layout, VkFormat format, bool clear)
{
	auto &subpassDescription = def.subpassDescriptions[def.currentSubpass];
	auto &attachment         = def.attachmentDescriptions.emplace_back();
	attachment.format        = format;
	attachment.samples       = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp        = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = layout.in;
	attachment.finalLayout   = layout.out;
	auto &ref                 = def.attachmentReferences.emplace_back();
	ref.attachment            = def.attachmentDescriptions.size() - 1;
	ref.layout                = layout.internal;
	switch (layout.internal)
	{
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			if (subpassDescription.colorAttachmentCount == 0)
			{
				subpassDescription.colorAttachmentCount = 1;
				subpassDescription.pColorAttachments    = &ref;
			}
			else
			{
				subpassDescription.colorAttachmentCount++;
			}
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			subpassDescription.pColorAttachments = &ref;
			break;
		default:
			printVka("Unsupported internal layout");
			DEBUG_BREAK;
			break;
	}
}
inline void addColorAttachment(RenderPassDefinition &def, VkImageLayout layoutIn, VkImageLayout layoutOut, VkFormat format, bool clear)
{
	addAttachment(def, {layoutIn, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, layoutOut}, format, clear);
}

inline void nextSubpass(RenderPassDefinition &def)
{
	auto &subpassDescription             = def.subpassDescriptions.emplace_back();
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	def.currentSubpass++;
}




}        // namespace vka
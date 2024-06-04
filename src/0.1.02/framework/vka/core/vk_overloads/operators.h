#pragma once
#include <vulkan/vulkan.h>
#include <vka/core/functions/misc.h>
#include <vka/core/functions/operators.h>
namespace vka
{

// RenderPassStuff
struct SubpassDescription_OP : public VkSubpassDescription
{
	bool operator==(SubpassDescription_OP const &other) const
	{
		// clang-format off
		return flags == other.flags
			&& pipelineBindPoint == other.pipelineBindPoint
			&& inputAttachmentCount == other.inputAttachmentCount
			&& shallowCmpStructures(pInputAttachments, other.pInputAttachments, inputAttachmentCount)
			&& colorAttachmentCount == other.colorAttachmentCount
			&& shallowCmpStructures(pColorAttachments, other.pColorAttachments, colorAttachmentCount)
			&& shallowCmpStructures(pResolveAttachments, other.pResolveAttachments, colorAttachmentCount)
			&& shallowCmpStructure(pDepthStencilAttachment, other.pDepthStencilAttachment)
			&& preserveAttachmentCount == other.preserveAttachmentCount
			&& shallowCmpStructures(pPreserveAttachments, other.pPreserveAttachments, preserveAttachmentCount);
		// clang-format on
	}
	bool operator!=(SubpassDescription_OP const &other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC pipelineBindPoint
			HASHC inputAttachmentCount
			HASHC hashArray(pInputAttachments, inputAttachmentCount)
			HASHC colorAttachmentCount
			HASHC hashArray(pColorAttachments, colorAttachmentCount)
			HASHC hashArray(pResolveAttachments, colorAttachmentCount)
			HASHC shallowHashStructure(pDepthStencilAttachment)
			HASHC preserveAttachmentCount
			HASHC hashArray(pPreserveAttachments,preserveAttachmentCount);
		// clang-format on
	}
};






} // namespace vka
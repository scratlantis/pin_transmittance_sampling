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
			&& memcmpArray(pInputAttachments, other.pInputAttachments, inputAttachmentCount)
			&& colorAttachmentCount == other.colorAttachmentCount
			&& memcmpArray(pColorAttachments, other.pColorAttachments, colorAttachmentCount)
			&& memcmpArray(pResolveAttachments, other.pResolveAttachments, colorAttachmentCount)
			&& memcmpPtr(pDepthStencilAttachment, other.pDepthStencilAttachment)
			&& preserveAttachmentCount == other.preserveAttachmentCount
			&& memcmpArray(pPreserveAttachments, other.pPreserveAttachments, preserveAttachmentCount);
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
			HASHC hashVector(pInputAttachments, inputAttachmentCount)
			HASHC colorAttachmentCount
			HASHC hashVector(pColorAttachments, colorAttachmentCount)
			HASHC hashVector(pResolveAttachments, colorAttachmentCount)
			HASHC byteHashPtr(pDepthStencilAttachment)
			HASHC preserveAttachmentCount
			HASHC hashVector(pPreserveAttachments,preserveAttachmentCount);
		// clang-format on
	}
};

struct PipelineMultisampleStateCreateInfo_OP : public VkPipelineMultisampleStateCreateInfo
{
	bool operator==(PipelineMultisampleStateCreateInfo_OP const &other) const
	{
		// clang-format off
		return flags == other.flags
			&& rasterizationSamples == other.rasterizationSamples
			&& sampleShadingEnable == other.sampleShadingEnable
			&& minSampleShading == other.minSampleShading
			&& alphaToCoverageEnable == other.alphaToCoverageEnable
			&& alphaToOneEnable == other.alphaToOneEnable;
		// clang-format on
	}
	bool operator!=(PipelineMultisampleStateCreateInfo_OP const &other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC rasterizationSamples
			HASHC sampleShadingEnable
			HASHC minSampleShading
			HASHC alphaToCoverageEnable
			HASHC alphaToOneEnable;
		// clang-format on
	}
};

DEFINE_ZERO_PAD(VkAttachmentDescription)
DEFINE_ZERO_PAD(VkAttachmentReference)
DEFINE_ZERO_PAD(VkSubpassDependency)
DEFINE_ZERO_PAD(VkDescriptorSetLayoutBinding)
DEFINE_ZERO_PAD(VkPushConstantRange)

DEFINE_ZERO_PAD(VkPipelineInputAssemblyStateCreateInfo)
DEFINE_ZERO_PAD(VkPipelineTessellationStateCreateInfo)
DEFINE_ZERO_PAD(VkPipelineRasterizationStateCreateInfo)
DEFINE_ZERO_PAD(VkPipelineDepthStencilStateCreateInfo)
DEFINE_ZERO_PAD(VkPipelineColorBlendAttachmentState)
DEFINE_ZERO_PAD(VkVertexInputBindingDescription)
DEFINE_ZERO_PAD(VkVertexInputAttributeDescription)
DEFINE_ZERO_PAD(VkViewport)
DEFINE_ZERO_PAD(VkRect2D)

} // namespace vka


DECLARE_HASH(vka::PipelineMultisampleStateCreateInfo_OP,hash)
DECLARE_HASH(vka::SubpassDescription_OP, hash)
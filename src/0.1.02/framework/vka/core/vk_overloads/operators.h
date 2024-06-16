#pragma once
#include <vulkan/vulkan.h>
#include <vka/core/functions/misc.h>
#include <vka/core/functions/operators.h>
namespace vka
{

// RenderPassStuff

struct VkAttachmentReference_OP : public VkAttachmentReference
{
	VkAttachmentReference_OP() = default;
	VkAttachmentReference_OP(VkAttachmentReference const &other)
	{
		this->attachment = other.attachment;
		this->layout     = other.layout;
	}
	bool operator==(VkAttachmentReference_OP const &other) const
	{
		// clang-format off
		return attachment == other.attachment
			&& layout == other.layout;
		// clang-format on
	}
	bool operator!=(VkAttachmentReference_OP const &other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return attachment
			HASHC layout;
		// clang-format on
	}
};
// DEFINE_ZERO_PAD(VkAttachmentReference)
// no resolve attachment
//struct SubpassDescription_OP : public VkSubpassDescription
//{
//	SubpassDescription_OP() = default;
//	SubpassDescription_OP(VkSubpassDescription const& other)
//	{
//		this->flags               = other.flags;
//		this->pipelineBindPoint   = other.pipelineBindPoint;
//		this->inputAttachmentCount = other.inputAttachmentCount;
//		this->pInputAttachments   = other.pInputAttachments;
//		this->colorAttachmentCount = other.colorAttachmentCount;
//		this->pColorAttachments    = other.pColorAttachments;
//		this->pResolveAttachments  = other.pResolveAttachments;
//		this->pDepthStencilAttachment = other.pDepthStencilAttachment;
//		this->preserveAttachmentCount = other.preserveAttachmentCount;
//		this->pPreserveAttachments = other.pPreserveAttachments;
//	}
//
//	bool operator==(SubpassDescription_OP const &other) const
//	{
//		// clang-format off
//		return PASS_TRUE(0,flags == other.flags																   )
//			&& PASS_TRUE(1,pipelineBindPoint == other.pipelineBindPoint										   )
//			&& PASS_TRUE(2,inputAttachmentCount == other.inputAttachmentCount									   )
//			&& PASS_TRUE(3,cmpArray(static_cast<const VkAttachmentReference_OP*>(pInputAttachments), static_cast<const VkAttachmentReference_OP*>(other.pInputAttachments), inputAttachmentCount)		   )
//			&& PASS_TRUE(4,colorAttachmentCount == other.colorAttachmentCount									   )
//			&& PASS_TRUE(5,cmpArray(static_cast<const VkAttachmentReference_OP*>(pColorAttachments), static_cast<const VkAttachmentReference_OP*>(other.pColorAttachments), colorAttachmentCount)		   )
//			&& PASS_TRUE(7,cmpPtr(static_cast<const VkAttachmentReference_OP*>(pDepthStencilAttachment), static_cast<const VkAttachmentReference_OP*>(other.pDepthStencilAttachment))					   )
//			&& PASS_TRUE(8,preserveAttachmentCount == other.preserveAttachmentCount							   )
//			&& PASS_TRUE(9,cmpArray(pPreserveAttachments,other.pPreserveAttachments, preserveAttachmentCount));
//		// clang-format on
//	}
//	bool operator!=(SubpassDescription_OP const &other) const
//	{
//		return !(*this == other);
//	}
//	bool hash() const
//	{
//		// clang-format off
//		return flags
//			HASHC pipelineBindPoint
//			HASHC inputAttachmentCount
//			HASHC hashVector(pInputAttachments, inputAttachmentCount)
//			HASHC colorAttachmentCount
//			HASHC hashVector(pColorAttachments, colorAttachmentCount)
//			HASHC byteHashPtr(pDepthStencilAttachment)
//			HASHC preserveAttachmentCount
//			HASHC hashVector(pPreserveAttachments,preserveAttachmentCount);
//		// clang-format on
//	}
//};

struct VkaSubpassDescription
{
	VkSubpassDescriptionFlags    flags;
	VkPipelineBindPoint          pipelineBindPoint;
	std::vector< VkAttachmentReference_OP> inputAttachments;
	std::vector<VkAttachmentReference_OP> colorAttachments;
	VkAttachmentReference_OP               depthStencilAttachment;
	bool hasDepthStencilAttachment = false;
	std::vector<uint32_t> preserveAttachments;
	VkaSubpassDescription() = default;
	VkaSubpassDescription(VkSubpassDescription const &other)
	{
		this->flags                   = other.flags;
		this->pipelineBindPoint       = other.pipelineBindPoint;
		this->inputAttachments.resize(other.inputAttachmentCount);
		for (size_t i = 0; i < other.inputAttachmentCount; i++)
		{
			this->inputAttachments[i] = other.pInputAttachments[i];
		}
		this->colorAttachments.resize(other.colorAttachmentCount);
		for (size_t i = 0; i < other.colorAttachmentCount; i++)
		{
			this->colorAttachments[i] = other.pColorAttachments[i];
		}
		if (other.pDepthStencilAttachment)
		{
			this->depthStencilAttachment = *other.pDepthStencilAttachment;
			hasDepthStencilAttachment = true;
		}
		else
		{
			this->depthStencilAttachment = {};
		}
		this->preserveAttachments.resize(other.preserveAttachmentCount);
		for (size_t i = 0; i < other.preserveAttachmentCount; i++)
		{
			this->preserveAttachments[i] = other.pPreserveAttachments[i];
		}
	}
	VkSubpassDescription getVulkanStruct() const
	{
		VkSubpassDescription subpassDescription{};
		subpassDescription.flags                   = flags;
		subpassDescription.pipelineBindPoint       = pipelineBindPoint;

		subpassDescription.inputAttachmentCount    = inputAttachments.size();
		subpassDescription.pInputAttachments       = inputAttachments.data();

		subpassDescription.colorAttachmentCount    = colorAttachments.size();
		subpassDescription.pColorAttachments    = colorAttachments.data();

		subpassDescription.pResolveAttachments     = nullptr;

		if (hasDepthStencilAttachment)
		{
			subpassDescription.pDepthStencilAttachment = &depthStencilAttachment;
		}
		else
		{
			subpassDescription.pDepthStencilAttachment = nullptr;
		}
		subpassDescription.preserveAttachmentCount = preserveAttachments.size();

		subpassDescription.pPreserveAttachments = preserveAttachments.data();
		return subpassDescription;
	}

	bool operator==(VkaSubpassDescription const &other) const
	{
		// clang-format off
		return PASS_TRUE(0,flags == other.flags)
			&& PASS_TRUE(1,pipelineBindPoint == other.pipelineBindPoint	)
			&& PASS_TRUE(3,cmpVector(inputAttachments, other.inputAttachments))
			&& PASS_TRUE(4,cmpVector(colorAttachments, other.colorAttachments))
			&& PASS_TRUE(5,depthStencilAttachment == other.depthStencilAttachment)
			&& PASS_TRUE(4,cmpVector(preserveAttachments, other.preserveAttachments));
		// clang-format on
	}
	bool operator!=(VkaSubpassDescription const &other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC pipelineBindPoint
			HASHC hashVector(inputAttachments)
			HASHC hashVector(colorAttachments)
			HASHC depthStencilAttachment.hash()
			HASHC hashVector(preserveAttachments);
		// clang-format on
	}
};

struct PipelineMultisampleStateCreateInfo_OP : public VkPipelineMultisampleStateCreateInfo
{
	PipelineMultisampleStateCreateInfo_OP() = default;
	PipelineMultisampleStateCreateInfo_OP(VkPipelineMultisampleStateCreateInfo const& other)
	{
		this->sType = other.sType;
		this->pNext = other.pNext;
		this->flags = other.flags;
		this->rasterizationSamples = other.rasterizationSamples;
		this->sampleShadingEnable = other.sampleShadingEnable;
		this->minSampleShading = other.minSampleShading;
		this->pSampleMask = other.pSampleMask;
		this->alphaToCoverageEnable = other.alphaToCoverageEnable;
		this->alphaToOneEnable = other.alphaToOneEnable;
	}
	bool operator==(PipelineMultisampleStateCreateInfo_OP const &other) const
	{
		// clang-format off
		return PASS_TRUE(0,flags == other.flags								 )
			&& PASS_TRUE(1,rasterizationSamples == other.rasterizationSamples	 )
			&& PASS_TRUE(2,sampleShadingEnable == other.sampleShadingEnable	 )
			&& PASS_TRUE(3,minSampleShading == other.minSampleShading			 )
			&& PASS_TRUE(4,alphaToCoverageEnable == other.alphaToCoverageEnable)
			&& PASS_TRUE(5,alphaToOneEnable == other.alphaToOneEnable);
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

struct VkRect2D_OP : public VkRect2D
{
	VkRect2D_OP() = default;
	VkRect2D_OP(VkRect2D const &other)
	{
		this->offset = other.offset;
		this->extent = other.extent;
	}
	bool operator==(VkRect2D_OP const &other) const
	{
		// clang-format off
		return offset.x == other.offset.x
			&& offset.y == other.offset.y
			&& extent.width == other.extent.width
			&& extent.height == other.extent.height;
		// clang-format on
	}
	bool operator!=(VkRect2D_OP const &other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return offset.x
			HASHC offset.y
			HASHC extent.width
			HASHC extent.height;
		// clang-format on
	}
};

struct VkAttachmentDescription_OP : public VkAttachmentDescription
{
	VkAttachmentDescription_OP() = default;
	VkAttachmentDescription_OP(VkAttachmentDescription const& other)
	{
		this->flags = other.flags;
		this->format = other.format;
		this->samples = other.samples;
		this->loadOp = other.loadOp;
		this->storeOp = other.storeOp;
		this->stencilLoadOp = other.stencilLoadOp;
		this->stencilStoreOp = other.stencilStoreOp;
		this->initialLayout = other.initialLayout;
		this->finalLayout = other.finalLayout;
	}
	bool operator==(VkAttachmentDescription_OP const& other) const
	{
		// clang-format off
		return flags == other.flags
			&& format == other.format
			&& samples == other.samples
			&& loadOp == other.loadOp
			&& storeOp == other.storeOp
			&& stencilLoadOp == other.stencilLoadOp
			&& stencilStoreOp == other.stencilStoreOp
			&& initialLayout == other.initialLayout
			&& finalLayout == other.finalLayout;
		// clang-format on
	}
	bool operator!=(VkAttachmentDescription_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC format
			HASHC samples
			HASHC loadOp
			HASHC storeOp
			HASHC stencilLoadOp
			HASHC stencilStoreOp
			HASHC initialLayout
			HASHC finalLayout;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkAttachmentDescription)



struct VkSubpassDependency_OP : public VkSubpassDependency
{
	VkSubpassDependency_OP() = default;
	VkSubpassDependency_OP(VkSubpassDependency const& other)
	{
		this->srcSubpass = other.srcSubpass;
		this->dstSubpass = other.dstSubpass;
		this->srcStageMask = other.srcStageMask;
		this->dstStageMask = other.dstStageMask;
		this->srcAccessMask = other.srcAccessMask;
		this->dstAccessMask = other.dstAccessMask;
		this->dependencyFlags = other.dependencyFlags;
	}
	bool operator==(VkSubpassDependency_OP const& other) const
	{
		// clang-format off
		return srcSubpass == other.srcSubpass
			&& dstSubpass == other.dstSubpass
			&& srcStageMask == other.srcStageMask
			&& dstStageMask == other.dstStageMask
			&& srcAccessMask == other.srcAccessMask
			&& dstAccessMask == other.dstAccessMask
			&& dependencyFlags == other.dependencyFlags;
		// clang-format on
	}
	bool operator!=(VkSubpassDependency_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return srcSubpass
			HASHC dstSubpass
			HASHC srcStageMask
			HASHC dstStageMask
			HASHC srcAccessMask
			HASHC dstAccessMask
			HASHC dependencyFlags;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkSubpassDependency)

struct VkDescriptorSetLayoutBinding_OP : public VkDescriptorSetLayoutBinding
{
	VkDescriptorSetLayoutBinding_OP() = default;
	VkDescriptorSetLayoutBinding_OP(VkDescriptorSetLayoutBinding const& other)
	{
		this->binding = other.binding;
		this->descriptorType = other.descriptorType;
		this->descriptorCount = other.descriptorCount;
		this->stageFlags = other.stageFlags;
	}
	bool operator==(VkDescriptorSetLayoutBinding_OP const& other) const
	{
		// clang-format off
		return binding == other.binding
			&& descriptorType == other.descriptorType
			&& descriptorCount == other.descriptorCount
			&& stageFlags == other.stageFlags;
		// clang-format on
	}
	bool operator!=(VkDescriptorSetLayoutBinding_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return binding
			HASHC descriptorType
			HASHC descriptorCount
			HASHC stageFlags;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkDescriptorSetLayoutBinding)

struct VkPushConstantRange_OP : public VkPushConstantRange
{
	VkPushConstantRange_OP() = default;
	VkPushConstantRange_OP(VkPushConstantRange const& other)
	{
		this->stageFlags = other.stageFlags;
		this->offset = other.offset;
		this->size = other.size;
	}
	bool operator==(VkPushConstantRange_OP const& other) const
	{
		// clang-format off
		return stageFlags == other.stageFlags
			&& offset == other.offset
			&& size == other.size;
		// clang-format on
	}
	bool operator!=(VkPushConstantRange_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return stageFlags
			HASHC offset
			HASHC size;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkPushConstantRange)

struct VkPipelineInputAssemblyStateCreateInfo_OP : public VkPipelineInputAssemblyStateCreateInfo
{
	VkPipelineInputAssemblyStateCreateInfo_OP() = default;
	VkPipelineInputAssemblyStateCreateInfo_OP(VkPipelineInputAssemblyStateCreateInfo const &other){
		this->sType = other.sType;
		this->pNext = other.pNext;
		this->flags = other.flags;
		this->topology = other.topology;
		this->primitiveRestartEnable = other.primitiveRestartEnable;
	}
	bool operator==(VkPipelineInputAssemblyStateCreateInfo_OP const& other) const
	{
		// clang-format off
		return flags == other.flags
			&& topology == other.topology
			&& primitiveRestartEnable == other.primitiveRestartEnable;
		// clang-format on
	}
	bool operator!=(VkPipelineInputAssemblyStateCreateInfo_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC topology
			HASHC primitiveRestartEnable;
		// clang-format on
	}
};
// DEFINE_ZERO_PAD(VkPipelineInputAssemblyStateCreateInfo)

struct VkPipelineTessellationStateCreateInfo_OP : public VkPipelineTessellationStateCreateInfo
{
	VkPipelineTessellationStateCreateInfo_OP() = default;
	VkPipelineTessellationStateCreateInfo_OP(VkPipelineTessellationStateCreateInfo const& other)
	{
		this->sType = other.sType;
		this->pNext = other.pNext;
		this->flags = other.flags;
		this->patchControlPoints = other.patchControlPoints;
	}
	bool operator==(VkPipelineTessellationStateCreateInfo_OP const& other) const
	{
		// clang-format off
		return flags == other.flags
			&& patchControlPoints == other.patchControlPoints;
		// clang-format on
	}
	bool operator!=(VkPipelineTessellationStateCreateInfo_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC patchControlPoints;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkPipelineTessellationStateCreateInfo)

struct VkPipelineRasterizationStateCreateInfo_OP : public VkPipelineRasterizationStateCreateInfo
{
	VkPipelineRasterizationStateCreateInfo_OP() = default;
	VkPipelineRasterizationStateCreateInfo_OP(VkPipelineRasterizationStateCreateInfo const& other)
	{
		this->sType = other.sType;
		this->pNext = other.pNext;
		this->flags = other.flags;
		this->depthClampEnable = other.depthClampEnable;
		this->rasterizerDiscardEnable = other.rasterizerDiscardEnable;
		this->polygonMode = other.polygonMode;
		this->cullMode = other.cullMode;
		this->frontFace = other.frontFace;
		this->depthBiasEnable = other.depthBiasEnable;
		this->depthBiasConstantFactor = other.depthBiasConstantFactor;
		this->depthBiasClamp = other.depthBiasClamp;
		this->depthBiasSlopeFactor = other.depthBiasSlopeFactor;
		this->lineWidth = other.lineWidth;
	}
	bool operator==(VkPipelineRasterizationStateCreateInfo_OP const& other) const
	{
		// clang-format off
		return flags == other.flags
			&& depthClampEnable == other.depthClampEnable
			&& rasterizerDiscardEnable == other.rasterizerDiscardEnable
			&& polygonMode == other.polygonMode
			&& cullMode == other.cullMode
			&& frontFace == other.frontFace
			&& depthBiasEnable == other.depthBiasEnable
			&& depthBiasConstantFactor == other.depthBiasConstantFactor
			&& depthBiasClamp == other.depthBiasClamp
			&& depthBiasSlopeFactor == other.depthBiasSlopeFactor
			&& lineWidth == other.lineWidth;
		// clang-format on
	}
	bool operator!=(VkPipelineRasterizationStateCreateInfo_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC depthClampEnable
			HASHC rasterizerDiscardEnable
			HASHC polygonMode
			HASHC cullMode
			HASHC frontFace
			HASHC depthBiasEnable
			HASHC depthBiasConstantFactor
			HASHC depthBiasClamp
			HASHC depthBiasSlopeFactor
			HASHC lineWidth;
		// clang-format on
	}
};
// DEFINE_ZERO_PAD(VkPipelineRasterizationStateCreateInfo)
struct VkStencilOpState_OP : public VkStencilOpState
{
	VkStencilOpState_OP() = default;
	VkStencilOpState_OP(VkStencilOpState const &other)
	{
		this->failOp      = other.failOp;
		this->passOp      = other.passOp;
		this->depthFailOp = other.depthFailOp;
		this->compareOp   = other.compareOp;
		this->compareMask = other.compareMask;
		this->writeMask   = other.writeMask;
		this->reference   = other.reference;
	}

	bool operator==(VkStencilOpState_OP const &other) const
	{
		// clang-format off
		return failOp == other.failOp
			&& passOp == other.passOp
			&& depthFailOp == other.depthFailOp
			&& compareOp == other.compareOp
			&& compareMask == other.compareMask
			&& writeMask == other.writeMask
			&& reference == other.reference;
		// clang-format on
	}
	bool operator!=(VkStencilOpState_OP const &other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return failOp
			HASHC passOp
			HASHC depthFailOp
			HASHC compareOp
			HASHC compareMask
			HASHC writeMask
			HASHC reference;
		// clang-format on
	}
};

struct VkPipelineDepthStencilStateCreateInfo_OP : public VkPipelineDepthStencilStateCreateInfo
{
	VkPipelineDepthStencilStateCreateInfo_OP() = default;
	VkPipelineDepthStencilStateCreateInfo_OP(VkPipelineDepthStencilStateCreateInfo const& other)
	{
		this->sType = other.sType;
		this->pNext = other.pNext;
		this->flags = other.flags;
		this->depthTestEnable = other.depthTestEnable;
		this->depthWriteEnable = other.depthWriteEnable;
		this->depthCompareOp = other.depthCompareOp;
		this->depthBoundsTestEnable = other.depthBoundsTestEnable;
		this->stencilTestEnable = other.stencilTestEnable;
		this->front = other.front;
		this->back = other.back;
		this->minDepthBounds = other.minDepthBounds;
		this->maxDepthBounds = other.maxDepthBounds;
	}
	bool operator==(VkPipelineDepthStencilStateCreateInfo_OP const& other) const
	{
		// clang-format off
		return flags == other.flags
			&& depthTestEnable == other.depthTestEnable
			&& depthWriteEnable == other.depthWriteEnable
			&& depthCompareOp == other.depthCompareOp
			&& depthBoundsTestEnable == other.depthBoundsTestEnable
			&& stencilTestEnable == other.stencilTestEnable
			&& VkStencilOpState_OP(front) ==  VkStencilOpState_OP(other.front)
			&& VkStencilOpState_OP(back) == VkStencilOpState_OP(other.back)
			&& minDepthBounds == other.minDepthBounds
			&& maxDepthBounds == other.maxDepthBounds;
		// clang-format on
	}
	bool operator!=(VkPipelineDepthStencilStateCreateInfo_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return flags
			HASHC depthTestEnable
			HASHC depthWriteEnable
			HASHC depthCompareOp
			HASHC depthBoundsTestEnable
			HASHC stencilTestEnable
			HASHC VkStencilOpState_OP(front)
			HASHC VkStencilOpState_OP(back)
			HASHC minDepthBounds
			HASHC maxDepthBounds;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkPipelineDepthStencilStateCreateInfo)




struct VkPipelineColorBlendAttachmentState_OP : public VkPipelineColorBlendAttachmentState
{
	VkPipelineColorBlendAttachmentState_OP() = default;
	VkPipelineColorBlendAttachmentState_OP(VkPipelineColorBlendAttachmentState const& other)
	{
		this->blendEnable = other.blendEnable;
		this->srcColorBlendFactor = other.srcColorBlendFactor;
		this->dstColorBlendFactor = other.dstColorBlendFactor;
		this->colorBlendOp = other.colorBlendOp;
		this->srcAlphaBlendFactor = other.srcAlphaBlendFactor;
		this->dstAlphaBlendFactor = other.dstAlphaBlendFactor;
		this->alphaBlendOp = other.alphaBlendOp;
		this->colorWriteMask = other.colorWriteMask;
	}
	bool operator==(VkPipelineColorBlendAttachmentState_OP const& other) const
	{
		// clang-format off
		return blendEnable == other.blendEnable
			&& srcColorBlendFactor == other.srcColorBlendFactor
			&& dstColorBlendFactor == other.dstColorBlendFactor
			&& colorBlendOp == other.colorBlendOp
			&& srcAlphaBlendFactor == other.srcAlphaBlendFactor
			&& dstAlphaBlendFactor == other.dstAlphaBlendFactor
			&& alphaBlendOp == other.alphaBlendOp
			&& colorWriteMask == other.colorWriteMask;
		// clang-format on
	}
	bool operator!=(VkPipelineColorBlendAttachmentState_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return blendEnable
			HASHC srcColorBlendFactor
			HASHC dstColorBlendFactor
			HASHC colorBlendOp
			HASHC srcAlphaBlendFactor
			HASHC dstAlphaBlendFactor
			HASHC alphaBlendOp
			HASHC colorWriteMask;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkPipelineColorBlendAttachmentState)

struct VkVertexInputBindingDescription_OP : public VkVertexInputBindingDescription
{
	VkVertexInputBindingDescription_OP() = default;
	VkVertexInputBindingDescription_OP(VkVertexInputBindingDescription const &other)
	{
		this->binding = other.binding;
		this->stride = other.stride;
		this->inputRate = other.inputRate;
	}
	bool operator==(VkVertexInputBindingDescription_OP const& other) const
	{
		// clang-format off
		return binding == other.binding
			&& stride == other.stride
			&& inputRate == other.inputRate;
		// clang-format on
	}
	bool operator!=(VkVertexInputBindingDescription_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return binding
			HASHC stride
			HASHC inputRate;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkVertexInputBindingDescription)


struct VkVertexInputAttributeDescription_OP : public VkVertexInputAttributeDescription
{
	VkVertexInputAttributeDescription_OP() = default;
	VkVertexInputAttributeDescription_OP(VkVertexInputAttributeDescription const& other)
	{
		this->location = other.location;
		this->binding = other.binding;
		this->format = other.format;
		this->offset = other.offset;
	}
	bool operator==(VkVertexInputAttributeDescription_OP const& other) const
	{
		// clang-format off
		return location == other.location
			&& binding == other.binding
			&& format == other.format
			&& offset == other.offset;
		// clang-format on
	}
	bool operator!=(VkVertexInputAttributeDescription_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return location
			HASHC binding
			HASHC format
			HASHC offset;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkVertexInputAttributeDescription)

struct VkViewport_OP : public VkViewport
{
	VkViewport_OP() = default;
	VkViewport_OP(VkViewport const& other)
	{
		this->x = other.x;
		this->y = other.y;
		this->width = other.width;
		this->height = other.height;
		this->minDepth = other.minDepth;
		this->maxDepth = other.maxDepth;
	}
	bool operator==(VkViewport_OP const& other) const
	{
		// clang-format off
		return x == other.x
			&& y == other.y
			&& width == other.width
			&& height == other.height
			&& minDepth == other.minDepth
			&& maxDepth == other.maxDepth;
		// clang-format on
	}
	bool operator!=(VkViewport_OP const& other) const
	{
		return !(*this == other);
	}
	bool hash() const
	{
		// clang-format off
		return x
			HASHC y
			HASHC width
			HASHC height
			HASHC minDepth
			HASHC maxDepth;
		// clang-format on
	}
};
//DEFINE_ZERO_PAD(VkViewport)
} // namespace vka


DECLARE_HASH(vka::PipelineMultisampleStateCreateInfo_OP,hash)
DECLARE_HASH(vka::VkaSubpassDescription, hash)
DECLARE_HASH(vka::VkRect2D_OP, hash)
DECLARE_HASH(vka::VkAttachmentDescription_OP, hash)
DECLARE_HASH(vka::VkAttachmentReference_OP, hash)
DECLARE_HASH(vka::VkSubpassDependency_OP, hash)
DECLARE_HASH(vka::VkDescriptorSetLayoutBinding_OP, hash)
DECLARE_HASH(vka::VkPushConstantRange_OP, hash)
DECLARE_HASH(vka::VkPipelineInputAssemblyStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkPipelineTessellationStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkPipelineRasterizationStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkPipelineDepthStencilStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkStencilOpState_OP, hash)
DECLARE_HASH(vka::VkPipelineColorBlendAttachmentState_OP, hash)
DECLARE_HASH(vka::VkVertexInputBindingDescription_OP, hash)
DECLARE_HASH(vka::VkVertexInputAttributeDescription_OP, hash)
DECLARE_HASH(vka::VkViewport_OP, hash)

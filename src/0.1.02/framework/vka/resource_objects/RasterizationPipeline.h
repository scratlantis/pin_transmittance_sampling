#pragma once
#include "DescriptorSetLayout.h"
#include "PipelineLayout.h"
#include "Shader.h"
#include "RenderPass.h"
#include "Resource.h"

namespace vka
{
struct GlobalColorBlendState
{
	GlobalColorBlendState()
	{
		memset(this, 0, sizeof(GlobalColorBlendState));
	}
	VkPipelineColorBlendStateCreateFlags flags;
	VkBool32                             logicOpEnable;
	VkLogicOp                            logicOp;
	float                                blendConstants[4];
};
class RasterizationPipelineDefinition : public ResourceIdentifier
{
  public:
	VkPipelineCreateFlags                            flags;
	uint32_t                                         subpass;
	PipelineMultisampleStateCreateInfo_OP            multisampleState;

	ZERO_PAD(VkPipelineInputAssemblyStateCreateInfo)          inputAssemblyState;
	ZERO_PAD(VkPipelineTessellationStateCreateInfo)          tessellationState;
	ZERO_PAD(VkPipelineRasterizationStateCreateInfo)           rasterizationState;
	ZERO_PAD(VkPipelineDepthStencilStateCreateInfo)            depthStencilState;
	GlobalColorBlendState                            globalColorBlendState;

	PipelineLayoutDefinition                         pipelineLayoutDefinition;
	RenderPassDefinition                             renderPassDefinition;

	std::vector<ShaderDefinition>                    shaderDefinitions;

	std::vector<VkDynamicState>                      dynamicStates;
	std::vector<VkSampleMask>                        sampleMasks;

	std::vector<ZERO_PAD(VkPipelineColorBlendAttachmentState)> colorBlendAttachmentStates;
	std::vector<ZERO_PAD(VkVertexInputBindingDescription)>     vertexBindingDescriptions;
	std::vector<ZERO_PAD(VkVertexInputAttributeDescription)>   vertexAttributeDescriptions;
	std::vector<ZERO_PAD(VkViewport)>                          viewports;
	std::vector<ZERO_PAD(VkRect2D)>                            scissors;

	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const RasterizationPipelineDefinition &other) const;
	hash_t hash() const override;
};

class RasterizationPipeline : public Cachable_T<VkPipeline>
{
  public:
	virtual void     free() override;
	RasterizationPipeline(RasterizationPipelineDefinition const &def);
};
}        // namespace vka
DECLARE_HASH(vka::RasterizationPipelineDefinition, hash)
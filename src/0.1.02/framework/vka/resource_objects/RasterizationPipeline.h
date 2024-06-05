#pragma once
#include "DescriptorSetLayout.h"
#include "PipelineLayout.h"
#include "Shader.h"
#include "RenderPass.h"
#include "Resource.h"
#include <vka/state_objects/global_state.h>

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
class RestarizationPipelineDefinition : public ResourceIdentifier
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



	hash_t hash() const;
	bool   _equals(ResourceIdentifier const &other) const override;
	bool   equals(RestarizationPipelineDefinition const &other) const;

  protected:
};

class RasterizationPipeline : public CachableResource
{
  private:
	VkPipeline handle;

  protected:
  public:
	virtual bool     _equals(Resource const &other) const override;
	virtual hash_t   hash() const override;
	virtual void     free() override;
	VkPipeline       getHandle() const;
	RasterizationPipeline(RestarizationPipelineDefinition const &def);
};
}        // namespace vka
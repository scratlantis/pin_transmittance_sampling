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
	VkPipelineColorBlendStateCreateFlags flags;
	VkBool32                             logicOpEnable;
	VkLogicOp                            logicOp;
	float                                blendConstants[4];
};
class RestarizationPipelineDefinition : public ResourceIdentifier
{
  public:
	VkPipelineCreateFlags                            flags;
	std::vector<ShaderDefinition>                    shaderDefinitions;
	std::vector<VkVertexInputBindingDescription>     vertexBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription>   vertexAttributeDescriptions;
	VkPipelineInputAssemblyStateCreateInfo           inputAssemblyState;
	VkPipelineTessellationStateCreateInfo            tessellationState;
	std::vector<VkViewport>                          viewport;
	std::vector<VkRect2D>                            scissor;
	VkPipelineRasterizationStateCreateInfo           rasterizationState;
	std::vector<VkSampleMask>                        sampleMasks;
	VkPipelineMultisampleStateCreateInfo             multisampleState;
	VkPipelineDepthStencilStateCreateInfo            depthStencilState;
	GlobalColorBlendState                            globalColorBlendState;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
	std::vector<VkDynamicState>                      dynamicStates;
	PipelineLayoutDefinition                         pipelineLayoutDefinition;
	RenderPassDefinition                             renderPassDefinition;
	uint32_t                                         subpass;



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
	RasterizationPipeline(RestarizationPipelineDefinition const &definition);
};
}        // namespace vka
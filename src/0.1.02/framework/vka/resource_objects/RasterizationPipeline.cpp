#include "RasterizationPipeline.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
hash_t RasterizationPipelineDefinition::hash() const
{
	// clang-format off
	// may remove parts for performance
	return
	flags
	HASHC subpass
	HASHC multisampleState

	HASHC byteHashPtr(&inputAssemblyState)
	HASHC byteHashPtr(&tessellationState)
	HASHC byteHashPtr(&rasterizationState)
	HASHC byteHashPtr(&depthStencilState)
	HASHC byteHashPtr(&globalColorBlendState)
	
	HASHC pipelineLayoutDefinition.hash()
	HASHC renderPassDefinition.hash()
	HASHC hashVector(shaderDefinitions)

	HASHC hashVector(dynamicStates)
	HASHC hashVector(sampleMasks)

	HASHC byteHashVector(colorBlendAttachmentStates)
	HASHC byteHashVector(shaderDefinitions)
	HASHC byteHashVector(vertexBindingDescriptions)
	HASHC byteHashVector(vertexAttributeDescriptions)
	HASHC byteHashVector(viewports)
	HASHC byteHashVector(scissors);
	// clang-format on
}

DEFINE_EQUALS_OVERLOAD(RasterizationPipelineDefinition, ResourceIdentifier)

bool RasterizationPipelineDefinition::operator==(const RasterizationPipelineDefinition &other) const
{
	// clang-format off
	return
	flags == other.flags
	&& subpass == other.subpass
	&& multisampleState == other.multisampleState

	&& memcmpPtr(&inputAssemblyState, &other.inputAssemblyState)
	&& memcmpPtr(&tessellationState, &other.tessellationState)
	&& memcmpPtr(&rasterizationState, &other.rasterizationState)
	&& memcmpPtr(&depthStencilState, &other.depthStencilState)
	&& memcmpPtr(&globalColorBlendState, &other.globalColorBlendState)
	
	&& pipelineLayoutDefinition == other.pipelineLayoutDefinition
	&& renderPassDefinition == other.renderPassDefinition
	&& cmpVector(shaderDefinitions, other.shaderDefinitions)

	&& memcmpArray(dynamicStates.data(), other.dynamicStates.data(), dynamicStates.size())
	&& memcmpArray(sampleMasks.data(), other.sampleMasks.data(), sampleMasks.size())

	&& memcmpVector(colorBlendAttachmentStates, other.colorBlendAttachmentStates)
	&& memcmpVector(shaderDefinitions, other.shaderDefinitions)
	&& memcmpVector(vertexBindingDescriptions, other.vertexBindingDescriptions)
	&& memcmpVector(vertexAttributeDescriptions, other.vertexAttributeDescriptions)
	&& memcmpVector(viewports, other.viewports)
	&& memcmpVector(scissors, other.scissors);
	// clang-format on
}

void RasterizationPipeline::free()
{
	vkDestroyPipeline(gState.device.logical, handle, nullptr);
}

static VkPipelineVertexInputStateCreateInfo makeVertexInputStateCI(
    std::vector<ZERO_PAD(VkVertexInputBindingDescription)> const   &bindingDescriptions,
    std::vector<ZERO_PAD(VkVertexInputAttributeDescription)> const &attributeDescriptions)
{
	VkPipelineVertexInputStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	ci.vertexBindingDescriptionCount   = VKA_COUNT(bindingDescriptions);
	ci.pVertexBindingDescriptions      = bindingDescriptions.data();
	ci.vertexAttributeDescriptionCount = VKA_COUNT(attributeDescriptions);
	ci.pVertexAttributeDescriptions    = attributeDescriptions.data();
	return ci;
}

static VkPipelineViewportStateCreateInfo makeViewportStateCI(
    std::vector<ZERO_PAD(VkViewport)> const &viewports,
    std::vector<ZERO_PAD(VkRect2D)> const   &scissors)
{
	VkPipelineViewportStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	ci.viewportCount = VKA_COUNT(viewports);
	ci.pViewports    = viewports.data();
	ci.scissorCount  = VKA_COUNT(scissors);
	ci.pScissors     = scissors.data();
	return ci;
}

static VkPipelineColorBlendStateCreateInfo makeColorBlendStateCI(
    std::vector<ZERO_PAD(VkPipelineColorBlendAttachmentState)> const &colorBlendAttachmentStates,
    GlobalColorBlendState const                                      &globalColorBlendState)
{
	VkPipelineColorBlendStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	ci.attachmentCount = VKA_COUNT(colorBlendAttachmentStates);
	ci.pAttachments    = colorBlendAttachmentStates.data();
	ci.logicOpEnable   = globalColorBlendState.logicOpEnable;
	ci.logicOp         = globalColorBlendState.logicOp;
	memcpy(ci.blendConstants, globalColorBlendState.blendConstants, sizeof(ci.blendConstants));
	return ci;
}        // namespace vka

static VkPipelineDynamicStateCreateInfo makeDynamicStateCI(std::vector<VkDynamicState> const &dynamicStates)
{
	VkPipelineDynamicStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	ci.dynamicStateCount = VKA_COUNT(dynamicStates);
	ci.pDynamicStates    = dynamicStates.data();
	return ci;
}

RasterizationPipeline::RasterizationPipeline(RasterizationPipelineDefinition const &def)
{
	std::vector <ZERO_PAD(VkViewport)> viewports;
	std::vector <ZERO_PAD(VkRect2D)> scissors;
	if (def.viewports.size() == 0 && def.scissors.size() == 0)
	{
		ZERO_PAD(VkViewport) viewport;
		viewport.width    = (float)gState.io.extent.width;
		viewport.height   = (float)gState.io.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewports.push_back(viewport);
		ZERO_PAD(VkRect2D) scissor;
		scissor.extent.width  = gState.io.extent.width;
		scissor.extent.height = gState.io.extent.height;
		scissors.push_back(scissor);
	}
	VkGraphicsPipelineCreateInfo ci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	ci.flags = def.flags;
	std::vector<VkPipelineShaderStageCreateInfo> stageCIs(def.shaderDefinitions.size());
	for (size_t i = 0; i < stageCIs.size(); i++)
	{
		stageCIs[i] = makeShaderStageCI(def.shaderDefinitions[i], pCache->fetch(def.shaderDefinitions[i]));
	}
	ci.stageCount                                           = VKA_COUNT(stageCIs);
	ci.pStages                                              = stageCIs.data();
	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = makeVertexInputStateCI(def.vertexBindingDescriptions, def.vertexAttributeDescriptions);
	ci.pVertexInputState                                    = &vertexInputStateCI;
	ci.pInputAssemblyState                                  = &def.inputAssemblyState;
	ci.pTessellationState                                   = &def.tessellationState;
	VkPipelineViewportStateCreateInfo viewportStateCI       = makeViewportStateCI(viewports, scissors);
	ci.pViewportState                                       = &viewportStateCI;
	ci.pRasterizationState                                  = &def.rasterizationState;
	ci.pMultisampleState                                    = &def.multisampleState;
	ci.pDepthStencilState                                   = &def.depthStencilState;
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI   = makeColorBlendStateCI(def.colorBlendAttachmentStates, def.globalColorBlendState);
	ci.pColorBlendState                                     = &colorBlendStateCI;
	VkPipelineDynamicStateCreateInfo dynamicStateCI         = makeDynamicStateCI(def.dynamicStates);
	ci.pDynamicState                                        = &dynamicStateCI;
	ci.layout                                               = pCache->fetch(def.pipelineLayoutDefinition);
	ci.renderPass                                           = pCache->fetch(def.renderPassDefinition);
	ci.subpass                                              = def.subpass;
	ci.basePipelineHandle                                   = nullptr;        // Dont support pipeline derivatives yet
	ci.basePipelineIndex                                    = -1;
	VK_CHECK(vkCreateGraphicsPipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
}



}        // namespace vka
#include "RasterizationPipeline.h"
#include "Shader.h"
namespace vka
{
hash_t RasterizationPipelineState::hash() const
{
	uint64_t hash = 0;

	for (size_t i = 0; i < shaderDefinitions.size(); i++)
	{
		hashCombine(hash, shaderDefinitions[i].hash());
	}
	for (size_t i = 0; i < vertexBindingDescription.size(); i++)
	{
		hashCombine(hash, vertexBindingDescription[i].binding);
		hashCombine(hash, vertexBindingDescription[i].inputRate);
		hashCombine(hash, vertexBindingDescription[i].stride);
	}
	for (size_t i = 0; i < vertexBindingDescription.size(); i++)
	{
		hashCombine(hash, vertexAttributeDescriptions[i].binding);
		hashCombine(hash, vertexAttributeDescriptions[i].format);
		hashCombine(hash, vertexAttributeDescriptions[i].location);
		hashCombine(hash, vertexAttributeDescriptions[i].offset);
	}
	hashCombine(hash, inputAssemblyState.topology);

	hashCombine(hash, viewport.height);
	hashCombine(hash, viewport.width);

	hashCombine(hash, rasterizationState.cullMode);
	hashCombine(hash, rasterizationState.frontFace);

	hashCombine(hash, depthStencilState.depthCompareOp);
	hashCombine(hash, depthStencilState.depthTestEnable);
	hashCombine(hash, depthStencilState.depthWriteEnable);

	for (size_t i = 0; i < blendAttachments.size(); i++)
	{
		hashCombine(hash, blendAttachments[i].alphaBlendOp);
		hashCombine(hash, blendAttachments[i].blendEnable);
		hashCombine(hash, blendAttachments[i].colorBlendOp);
		hashCombine(hash, blendAttachments[i].colorWriteMask);
		hashCombine(hash, blendAttachments[i].dstAlphaBlendFactor);
		hashCombine(hash, blendAttachments[i].dstColorBlendFactor);
		hashCombine(hash, blendAttachments[i].srcAlphaBlendFactor);
		hashCombine(hash, blendAttachments[i].srcColorBlendFactor);
	}

	hashCombine(hash, layout);

	hashCombine(hash, shallowHashArray(specialisationEntryCounts));
	hashCombine(hash, shallowHashArray(specialisationEntrySizes));
	hashCombine(hash, shallowHashArray(specialisationData));

#ifdef HASH_STATIC_PIPELINE_STATES
	for (size_t i = 0; i < dynamicStates.size(); i++)
	{
		hashCombine(hash, dynamicStates[i]);
	}
	hashCombine(hash, dynamicState.flags);

	hashCombine(hash, rasterizationState.depthBiasClamp);
	hashCombine(hash, rasterizationState.depthBiasConstantFactor);
	hashCombine(hash, rasterizationState.depthBiasEnable);
	hashCombine(hash, rasterizationState.depthBiasSlopeFactor);
	hashCombine(hash, rasterizationState.depthClampEnable);
	hashCombine(hash, rasterizationState.flags);
	hashCombine(hash, rasterizationState.lineWidth);
	hashCombine(hash, rasterizationState.polygonMode);
	hashCombine(hash, rasterizationState.rasterizerDiscardEnable);

	hashCombine(hash, vertexInputState.flags);
	hashCombine(hash, inputAssemblyState.flags);
	hashCombine(hash, inputAssemblyState.primitiveRestartEnable);

	hashCombine(hash, colorBlendState.blendConstants[0]);
	hashCombine(hash, colorBlendState.blendConstants[1]);
	hashCombine(hash, colorBlendState.blendConstants[2]);
	hashCombine(hash, colorBlendState.blendConstants[3]);
	hashCombine(hash, colorBlendState.flags);
	hashCombine(hash, colorBlendState.logicOp);
	hashCombine(hash, colorBlendState.logicOpEnable);

	hashCombine(hash, viewportState.flags);
	hashCombine(hash, viewport.maxDepth);
	hashCombine(hash, viewport.minDepth);
	hashCombine(hash, viewport.x);
	hashCombine(hash, viewport.y);

	hashCombine(hash, depthStencilState.back);
	hashCombine(hash, depthStencilState.depthBoundsTestEnable);
	hashCombine(hash, depthStencilState.flags);
	hashCombine(hash, depthStencilState.front);
	hashCombine(hash, depthStencilState.stencilTestEnable);
	hashCombine(hash, depthStencilState.maxDepthBounds);
	hashCombine(hash, depthStencilState.minDepthBounds);
#endif        // HASH_STATIC_PIPELINE_STATES

#ifdef HASH_UNUSED_PIPELINE_STATES

	hashCombine(hash, multisampleState.alphaToCoverageEnable);
	hashCombine(hash, multisampleState.alphaToOneEnable);
	hashCombine(hash, multisampleState.flags);
	hashCombine(hash, multisampleState.minSampleShading);
	if (multisampleState.pSampleMask != nullptr)
	{
		hashCombine(hash, multisampleState.pSampleMask);
	}
	hashCombine(hash, multisampleState.rasterizationSamples);
	hashCombine(hash, multisampleState.sampleShadingEnable);
#endif        // HASH_UNUSED_PIPELINE_STATES

	return hash;
}

bool RasterizationPipelineState::_equals(RasterizationPipelineState const &other) const
{
	// clang-format off
	/*bool tmp1 = cmpArray(shaderDefinitions, other.shaderDefinitions)										   ;
	bool tmp2= shallowCmpArray(vertexBindingDescription, other.vertexBindingDescription)					   ;
	bool tmp3 = shallowCmpArray(vertexAttributeDescriptions, other.vertexAttributeDescriptions)				   ;
	bool tmp4 = inputAssemblyState.topology == other.inputAssemblyState.topology							   ;
	bool tmp5 = viewport.height == other.viewport.height													   ;
	bool tmp6 = viewport.width == other.viewport.width														   ;
	bool tmp7 = rasterizationState.cullMode == other.rasterizationState.cullMode							   ;
	bool tmp8 = rasterizationState.frontFace == other.rasterizationState.frontFace							   ;
	bool tmp9 = depthStencilState.depthCompareOp == other.depthStencilState.depthCompareOp					   ;
	bool tmp10 = depthStencilState.depthTestEnable == other.depthStencilState.depthTestEnable				   ;
	bool tmp11= depthStencilState.depthWriteEnable == other.depthStencilState.depthWriteEnable				   ;
	bool tmp12 = shallowCmpArray(blendAttachments, other.blendAttachments)									   ;
	bool tmp13 = layout == other.layout																		   ;
	bool tmp14 = shallowCmpArray(dynamicStates, other.dynamicStates)										   ;
	bool tmp15 = dynamicState.flags == other.dynamicState.flags												   ;
	bool tmp16 = rasterizationState.depthBiasClamp == other.rasterizationState.depthBiasClamp				   ;
	bool tmp17 = rasterizationState.depthBiasConstantFactor == other.rasterizationState.depthBiasConstantFactor;
	bool tmp18 = rasterizationState.depthBiasEnable == other.rasterizationState.depthBiasEnable				   ;
	bool tmp19 = rasterizationState.depthBiasSlopeFactor == other.rasterizationState.depthBiasSlopeFactor	   ;
	bool tmp20 = rasterizationState.depthClampEnable == other.rasterizationState.depthClampEnable			   ;
	bool tmp21 = rasterizationState.flags == other.rasterizationState.flags									   ;
	bool tmp22 = rasterizationState.lineWidth == other.rasterizationState.lineWidth							   ;
	bool tmp23 = rasterizationState.polygonMode == other.rasterizationState.polygonMode						   ;
	bool tmp24 = rasterizationState.rasterizerDiscardEnable == other.rasterizationState.rasterizerDiscardEnable;
	bool tmp25 = vertexInputState.flags == other.vertexInputState.flags										   ;
	bool tmp26 = inputAssemblyState.flags == other.inputAssemblyState.flags									   ;
	bool tmp27 = inputAssemblyState.primitiveRestartEnable == other.inputAssemblyState.primitiveRestartEnable  ;
	bool tmp28 = colorBlendState.blendConstants[0] == other.colorBlendState.blendConstants[0]				   ;
	bool tmp29 = colorBlendState.blendConstants[1] == other.colorBlendState.blendConstants[1]				   ;
	bool tmp30 = colorBlendState.blendConstants[2] == other.colorBlendState.blendConstants[2]				   ;
	bool tmp31 = colorBlendState.blendConstants[3] == other.colorBlendState.blendConstants[3]				   ;
	bool tmp32 = colorBlendState.flags == other.colorBlendState.flags										   ;
	bool tmp33 = colorBlendState.logicOp == other.colorBlendState.logicOp									   ;
	bool tmp34 = colorBlendState.logicOpEnable == other.colorBlendState.logicOpEnable						   ;
	bool tmp35 = viewportState.flags == other.viewportState.flags											   ;
	bool tmp36 = viewport.maxDepth == other.viewport.maxDepth												   ;
	bool tmp37 = viewport.minDepth == other.viewport.minDepth												   ;
	bool tmp38 = viewport.x == other.viewport.x &&viewport.y == other.viewport.y							   ;
	bool tmp39 = shallowCmpStructure(&depthStencilState.back, &other.depthStencilState.back)				   ;
	bool tmp40 = depthStencilState.depthBoundsTestEnable == other.depthStencilState.depthBoundsTestEnable	   ;
	bool tmp41 = depthStencilState.flags == other.depthStencilState.flags									   ;
	bool tmp42 = shallowCmpStructure(&depthStencilState.front, &other.depthStencilState.front)				   ;
	bool tmp43 = depthStencilState.stencilTestEnable == other.depthStencilState.stencilTestEnable			   ;
	bool tmp44 = depthStencilState.maxDepthBounds == other.depthStencilState.maxDepthBounds					   ;
	bool tmp45 = depthStencilState.minDepthBounds == other.depthStencilState.minDepthBounds					   ;
	bool tmp46 = multisampleState.alphaToCoverageEnable == other.multisampleState.alphaToCoverageEnable		   ;
	bool tmp47 = multisampleState.alphaToOneEnable == other.multisampleState.alphaToOneEnable				   ;
	bool tmp48 = multisampleState.flags == other.multisampleState.flags										   ;
	bool tmp49 = multisampleState.minSampleShading == other.multisampleState.minSampleShading				   ;
	bool tmp50 = multisampleState.rasterizationSamples == other.multisampleState.rasterizationSamples		   ;
	bool tmp51 = multisampleState.sampleShadingEnable == other.multisampleState.sampleShadingEnable			   ;
	bool tmp52 = shallowCmpStructure(multisampleState.pSampleMask, other.multisampleState.pSampleMask)		   ;
	bool tmp53 = shallowCmpArray(specialisationEntryCounts, other.specialisationEntryCounts)				   ;
	bool tmp54 = shallowCmpArray(specialisationEntrySizes, other.specialisationEntrySizes)					   ;
	bool tmp55 = shallowCmpArray(specialisationData, other.specialisationData)								   ;*/
	return cmpArray(shaderDefinitions, other.shaderDefinitions)
		&& shallowCmpArray(vertexBindingDescription, other.vertexBindingDescription)
		&& shallowCmpArray(vertexAttributeDescriptions, other.vertexAttributeDescriptions)
		&& inputAssemblyState.topology == other.inputAssemblyState.topology
		&& viewport.height == other.viewport.height
		&& viewport.width == other.viewport.width
		&& rasterizationState.cullMode == other.rasterizationState.cullMode
		&& rasterizationState.frontFace == other.rasterizationState.frontFace
		&& depthStencilState.depthCompareOp == other.depthStencilState.depthCompareOp
		&& depthStencilState.depthTestEnable == other.depthStencilState.depthTestEnable
		&& depthStencilState.depthWriteEnable == other.depthStencilState.depthWriteEnable
		&& shallowCmpArray(blendAttachments, other.blendAttachments)
		&& layout == other.layout
		&& shallowCmpArray(dynamicStates, other.dynamicStates)
		&& dynamicState.flags == other.dynamicState.flags
		&& rasterizationState.depthBiasClamp == other.rasterizationState.depthBiasClamp
		&& rasterizationState.depthBiasConstantFactor == other.rasterizationState.depthBiasConstantFactor
		&& rasterizationState.depthBiasEnable == other.rasterizationState.depthBiasEnable
		&& rasterizationState.depthBiasSlopeFactor == other.rasterizationState.depthBiasSlopeFactor
		&& rasterizationState.depthClampEnable == other.rasterizationState.depthClampEnable
		&& rasterizationState.flags == other.rasterizationState.flags
		&& rasterizationState.lineWidth == other.rasterizationState.lineWidth
		&& rasterizationState.polygonMode == other.rasterizationState.polygonMode
		&& rasterizationState.rasterizerDiscardEnable == other.rasterizationState.rasterizerDiscardEnable
		&& vertexInputState.flags == other.vertexInputState.flags
		&& inputAssemblyState.flags == other.inputAssemblyState.flags
		&& inputAssemblyState.primitiveRestartEnable == other.inputAssemblyState.primitiveRestartEnable
		&& colorBlendState.blendConstants[0] == other.colorBlendState.blendConstants[0]
		&& colorBlendState.blendConstants[1] == other.colorBlendState.blendConstants[1]
		&& colorBlendState.blendConstants[2] == other.colorBlendState.blendConstants[2]
		&& colorBlendState.blendConstants[3] == other.colorBlendState.blendConstants[3]
		&& colorBlendState.flags == other.colorBlendState.flags
		&& colorBlendState.logicOp == other.colorBlendState.logicOp
		&& colorBlendState.logicOpEnable == other.colorBlendState.logicOpEnable
		&& viewportState.flags == other.viewportState.flags
		&& viewport.maxDepth == other.viewport.maxDepth
		&& viewport.minDepth == other.viewport.minDepth
		&& viewport.x == other.viewport.x &&viewport.y == other.viewport.y
		&& shallowCmpStructure(&depthStencilState.back, &other.depthStencilState.back)
		&& depthStencilState.depthBoundsTestEnable == other.depthStencilState.depthBoundsTestEnable
		&& depthStencilState.flags == other.depthStencilState.flags
		&& shallowCmpStructure(&depthStencilState.front, &other.depthStencilState.front)
		&& depthStencilState.stencilTestEnable == other.depthStencilState.stencilTestEnable
		&& depthStencilState.maxDepthBounds == other.depthStencilState.maxDepthBounds
		&& depthStencilState.minDepthBounds == other.depthStencilState.minDepthBounds
		&& multisampleState.alphaToCoverageEnable == other.multisampleState.alphaToCoverageEnable
		&& multisampleState.alphaToOneEnable == other.multisampleState.alphaToOneEnable
		&& multisampleState.flags == other.multisampleState.flags
		&& multisampleState.minSampleShading == other.multisampleState.minSampleShading
		&& multisampleState.rasterizationSamples == other.multisampleState.rasterizationSamples
		&& multisampleState.sampleShadingEnable == other.multisampleState.sampleShadingEnable
		&& shallowCmpStructure(multisampleState.pSampleMask, other.multisampleState.pSampleMask)
		&& shallowCmpArray(specialisationEntryCounts, other.specialisationEntryCounts)
		&& shallowCmpArray(specialisationEntrySizes, other.specialisationEntrySizes)
		&& shallowCmpArray(specialisationData, other.specialisationData);
	// clang-format on
}

VkGraphicsPipelineCreateInfo RasterizationPipelineState::buildPipelineCI(ResourceTracker *pTracker,VkRenderPass renderPass, uint32_t subpassIndex)
{
	stages.clear();
	for (size_t i = 0; i < shaderDefinitions.size(); i++)
	{
		stages.push_back(Shader(pTracker, shaderDefinitions[i]).getStageCI());
	}

	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates    = dynamicStates.data();
	
	colorBlendState.attachmentCount = blendAttachments.size();
	colorBlendState.pAttachments    = blendAttachments.data();
	
	vertexInputState.vertexBindingDescriptionCount   = vertexBindingDescription.size();
	vertexInputState.pVertexBindingDescriptions      = vertexBindingDescription.data();
	vertexInputState.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
	vertexInputState.pVertexAttributeDescriptions    = vertexAttributeDescriptions.data();
	
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	pipelineCreateInfo.stageCount          = stages.size();
	pipelineCreateInfo.pStages             = stages.data();
	pipelineCreateInfo.pVertexInputState   = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pTessellationState  = nullptr;
	pipelineCreateInfo.pViewportState      = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pMultisampleState   = &multisampleState;
	pipelineCreateInfo.pDepthStencilState  = &depthStencilState;
	pipelineCreateInfo.pColorBlendState    = &colorBlendState;
	pipelineCreateInfo.pDynamicState       = &dynamicState;
	pipelineCreateInfo.layout              = PipelineLayout(pTracker, layout).getHandle();
	pipelineCreateInfo.renderPass          = renderPass;
	pipelineCreateInfo.subpass             = subpassIndex;
	pipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex   = -1;
	
	return pipelineCreateInfo;
}

// Default constructor
RasterizationPipelineState::RasterizationPipelineState()
{
	vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	inputAssemblyState                        = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	inputAssemblyState.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	viewport          = {};
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	scissor           = {};

	viewportState               = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewportState.viewportCount = 1;
	viewportState.pViewports    = &viewport;
	viewportState.scissorCount  = 1;
	viewportState.pScissors     = &scissor;

	rasterizationState                         = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	rasterizationState.depthClampEnable        = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode             = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode                = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable         = VK_FALSE;
	rasterizationState.lineWidth               = 1.0f;

	multisampleState                      = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading     = 1.0f;

	depthStencilState                = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;

	colorBlendState               = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp       = VK_LOGIC_OP_NO_OP;

	dynamicStates = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR};

	dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
}
void RasterizationPipelineState::addShaderDefinitions(ShaderDefinition shaderDef)
{
	shaderDefinitions.push_back(shaderDef);
}

void RasterizationPipelineState::addVertexBinding(VkVertexInputBindingDescription desc)
{
	vertexBindingDescription.push_back(desc);
}

RasterizationPipelineState &RasterizationPipelineState::setVertexAttributes(std::vector<VkVertexInputAttributeDescription> vertexAttributes)
{
	vertexAttributeDescriptions = vertexAttributes;
	return *this;
}

RasterizationPipelineState &RasterizationPipelineState::setPrimitiveTopology(VkPrimitiveTopology topology)
{
	inputAssemblyState.topology = topology;
	return *this;
}

RasterizationPipelineState &RasterizationPipelineState::setExtent(uint32_t width, uint32_t height)
{
	viewport.width        = width;
	viewport.height       = height;
	scissor.extent.width  = width;
	scissor.extent.height = height;
	return *this;
}

RasterizationPipelineState &RasterizationPipelineState::setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE)
{
	rasterizationState.cullMode  = cullMode;
	rasterizationState.frontFace = frontFace;
	return *this;
}
RasterizationPipelineState &RasterizationPipelineState::enableDepthTest(VkCompareOp compareOp, VkBool32 enableDepthWrite)
{
	depthStencilState.depthTestEnable  = VK_TRUE;
	depthStencilState.depthWriteEnable = enableDepthWrite;
	depthStencilState.depthCompareOp   = compareOp;
	return *this;
}
RasterizationPipelineState &RasterizationPipelineState::disableDepthTest()
{
	depthStencilState.depthTestEnable  = VK_FALSE;
	depthStencilState.depthWriteEnable = VK_FALSE;
	return *this;
}

RasterizationPipelineState &RasterizationPipelineState::setDescriptorLayout(DescriptorSetLayoutDefinition layout)
{
	this->layout.descSetLayoutDef = {layout};
	return *this;
}

VkPipelineColorBlendAttachmentState RasterizationPipelineState::getBlendAttachment(BlendMode blendMode)
{
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable                         = VK_TRUE;
	colorBlendAttachment.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp                        = VK_BLEND_OP_ADD;

	colorBlendAttachment.srcColorBlendFactor = blendMode.srcFactor;
	colorBlendAttachment.dstColorBlendFactor = blendMode.dstFactor;
	colorBlendAttachment.colorBlendOp        = blendMode.blendOp;
	return colorBlendAttachment;
}

void RasterizationPipelineState::addBlendMode(uint32_t attachmentCnt, BlendMode blendMode)
{
	for (size_t i = 0; i < attachmentCnt; i++)
	{
		blendAttachments.push_back(getBlendAttachment(blendMode));
	}
}


RasterizationPipeline::RasterizationPipeline(ResourceTracker* pTracker, const RasterizationPipelineState pipelineState, VkRenderPass renderPass, uint32_t subpassIndex) :
    UniqueResource(pTracker), pipelineState(pipelineState), renderPass(renderPass), subpassIndex(subpassIndex)
{
}

RasterizationPipeline::~RasterizationPipeline()
{
}

}        // namespace vka
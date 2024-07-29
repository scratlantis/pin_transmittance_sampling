#pragma once
#include <vka/app_state/AppState.h>
#include <vka/core_interface/types.h>
#include <vka/core/resources/cachable/RasterizationPipeline.h>
#include <vka/core/resources/cachable/ComputePipeline.h>
#include <vka/core/resources/Descriptor.h>
namespace vka
{

struct DrawCmd
{
	DrawSurface                          surf;
	RasterizationPipelineDefinition		 pipelineDef;
	std::vector<Image>                   attachments;
	std::vector<Descriptor>              descriptors;
	std::vector<ClearValue>              clearValues;
	VkRect2D_OP                          renderArea;
	std::vector<Buffer>                  instanceBuffers;
	uint32_t                             instanceCount;
	Buffer								 pushConstantsData;
	std::vector<uint32_t>                pushConstantsSizes;

	RenderState getRenderState() const;
};

 struct ComputeCmd
{
	 ComputePipelineDefinition pipelineDef;
	 std::vector<Descriptor>   descriptors;
	 glm::uvec3                workGroupCount;
	 Buffer                    pushConstantsData;
	 std::vector<uint32_t>     pushConstantsSizes;
	 RenderState          getRenderState() const;
 };

struct RasterizationPipelineInitValues
{
	VkPipelineCreateFlags       flags;
	VkSampleCountFlagBits       sampleCount;
	float                       minSampleShading;
	VkPrimitiveTopology         primitiveTopology;
	float                       lineWidth;
	float                       minDepthBounds;
	float                       maxDepthBounds;
	VkLogicOp                   blendLogicOp;
	std::vector<VkDynamicState> dynamicStates;
};

struct VertexDataLayout
{
	std::vector<VkFormat> formats;
	std::vector<uint32_t> offsets;
	uint32_t              stride;
};

struct BlendOperation
{
	VkBlendFactor srcFactor;
	VkBlendFactor dstFactor;
	VkBlendOp     op;
};
}        // namespace vka

#define VKA_BLEND_OP_WRITE                                         \
	{                                                              \
		VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD \
	}
#define VKA_BLEND_OP_ALPHA                                                              \
	{                                                                                   \
		VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD \
	}
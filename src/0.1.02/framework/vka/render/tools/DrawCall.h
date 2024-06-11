#pragma once
#include <vka/resource_objects/resource_common.h>
#include <vka/interface/types.h>
#include <vka/render/tools/ModelCache.h>
namespace vka
{


enum RenderStateActionBits
{
	RENDER_STATE_ACTION_BIT_START_RENDER_PASS  = 1 << 0,
	RENDER_STATE_ACTION_BIT_END_RENDER_PASS    = 1 << 1,
	RENDER_STATE_ACTION_BIT_NEXT_SUBPASS       = 1 << 2,
	RENDER_STATE_ACTION_BIT_BIND_PIPELINE      = 1 << 3,
	RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER = 1 << 4,
	RENDER_STATE_ACTION_BIT_BIND_INDEX_BUFFER  = 1 << 5,
};

struct RenderState
{
	// -> RenderPassBegin
	VkFramebuffer           framebuffer;
	std::vector<ClearValue> clearValues;
	VkRect2D_OP             renderArea;
	VkRenderPass            renderPass;

	// -> Subpass
	uint32_t subpassIdx;

	// -> BindPipeline
	VkPipeline pipeline;
	PipelineLayoutDefinition pipelineLayoutDef;
	VkPipelineBindPoint      bindPoint;

	// -> Bind Buffers
	std::vector<VkaBuffer> vertexBuffers;
	VkaBuffer indexBuffer;

	uint32_t calculateDifferenceBits(const RenderState &other) const
	{
		uint32_t diffBits = 0;
		// todo subpasses
		if (renderPass == VK_NULL_HANDLE)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_START_RENDER_PASS;
		}
		else if (other.renderPass != renderPass || cmpVector(clearValues, other.clearValues) || renderArea != other.renderArea)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_END_RENDER_PASS;
			diffBits |= RENDER_STATE_ACTION_BIT_START_RENDER_PASS;
		}
		if (pipeline != other.pipeline)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_PIPELINE;
		}
		if (!cmpVector( vertexBuffers , other.vertexBuffers))
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER;
		}
		if (indexBuffer != other.indexBuffer)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_INDEX_BUFFER;
		}
		return diffBits;
	}

	std::vector<VkClearValue> getClearValues() const;
};

struct DrawCall
{
	ModelData model;
	RasterizationPipelineDefinition pipelineDef;
	VkFramebuffer framebuffer;
	std::vector<IDescriptor *> descriptors;
	std::vector<ClearValue> clearValues;
	VkRect2D_OP                 renderArea;
	std::vector<VkaBuffer> instanceBuffers;
	uint32_t instanceCount;

	RenderState               getRenderState() const;
	
};

}
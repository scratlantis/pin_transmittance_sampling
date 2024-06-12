#pragma once
#include <vka/resource_objects/resource_common.h>
#include <vka/state_objects/global_state.h>
typedef vka::Buffer_I    *VkaBuffer;
typedef vka::Image_I     *VkaImage;
typedef vka::CmdBuffer_I *VkaCommandBuffer;
typedef vka::AppState      *VkaState;

class DescriptorObjects; // sampler, image , buffer


struct ImageSubresourceRange
{
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
};

typedef uint32_t Index;

struct SurfaceData
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
};

struct ModelData
{
	VkaBuffer vertexBuffer;
	VkaBuffer indexBuffer;
	VkaBuffer surfaceBuffer;
	uint32_t  surfaceCount;

	bool operator==(const ModelData &other) const
	{
		return vertexBuffer == other.vertexBuffer && indexBuffer == other.indexBuffer && surfaceBuffer == other.surfaceBuffer && surfaceCount == other.surfaceCount;
	};
	vka::hash_t hash() const
	{
		return vertexBuffer HASHC indexBuffer HASHC surfaceBuffer HASHC surfaceCount;
	}
};

struct DrawCmd
{
	ModelData                       model;
	vka::RasterizationPipelineDefinition pipelineDef;
	std::vector<VkaImage>           attachments;
	VkFramebuffer                   framebuffer;
	std::vector<vka::IDescriptor *>      descriptors;
	std::vector<vka::ClearValue>         clearValues;
	vka::VkRect2D_OP                     renderArea;
	std::vector<VkaBuffer>          instanceBuffers;
	uint32_t                        instanceCount;

	vka::RenderState getRenderState() const;
};

struct ComputeCmd
{
	vka::ComputePipelineDefinition pipelineDef;
	std::vector<VkaImage>      attachments;
	std::vector<vka::IDescriptor *> descriptors;

	vka::RenderState getRenderState() const;
};
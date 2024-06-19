#pragma once
#include <vka/resource_objects/resource_common.h>
#include <vka/state_objects/global_state.h>
typedef vka::Buffer_I       *VkaBuffer;
typedef vka::Image_I        *VkaImage;
typedef vka::CmdBuffer_I    *VkaCommandBuffer;
typedef vka::AppState       *VkaState;
typedef const vka::Buffer_R *Mappable;

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

struct ModelKey
{
	std::string path;
	void       *loadFunction;

	bool operator==(const ModelKey &other) const
	{
		return path == other.path && loadFunction == loadFunction;
	}

	vka::hash_t hash() const
	{
		return std::hash<std::string>()(path) ^ std::hash<void *>()(loadFunction);
	}
};
DECLARE_HASH(ModelKey, hash);


namespace std
{

template <>
struct hash<glm::vec3>
{
	size_t operator()(glm::vec3 const &type) const
	{
		return type.x HASHC type.y HASHC type.z;
	}
};

template <>
struct hash<glm::vec2>
{
	size_t operator()(glm::vec2 const &type) const
	{
		return type.x HASHC type.y;
	};
};

}        // namespace std

struct ObjVertex
{
	glm::vec3 v;
	glm::vec2 vt;
	glm::vec3 vn;

	ObjVertex(glm::vec3 v, glm::vec2 vt, glm::vec3 vn) :
	    v(v), vt(vt), vn(vn)
	{}
	ObjVertex() :
	    v(0), vt(0), vn(0)
	{}
	bool operator==(const ObjVertex &other) const
	{
		return v == other.v && vt == other.vt && vn == other.vn;
	}
	vka::hash_t hash() const
	{
		return std::hash<glm::vec3>()(v) ^ std::hash<glm::vec2>()(vt) ^ std::hash<glm::vec3>()(vn);
	}
};


DECLARE_HASH(ObjVertex, hash);

struct DrawCmd
{
	ModelData                            model;
	vka::RasterizationPipelineDefinition pipelineDef;
	std::vector<VkaImage>                attachments;
	VkFramebuffer                        framebuffer;
	std::vector<vka::IDescriptor *>      descriptors;
	std::vector<vka::ClearValue>         clearValues;
	vka::VkRect2D_OP                     renderArea;
	std::vector<VkaBuffer>               instanceBuffers;
	uint32_t                             instanceCount;

	vka::RenderState getRenderState() const;
};

struct ComputeCmd
{
	vka::ComputePipelineDefinition pipelineDef;
	std::vector<vka::IDescriptor *> descriptors;
	glm::uvec3 workGroupCount;
	vka::RenderState getRenderState() const;
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
	uint32_t stride;
};

struct BlendOperation
{
	VkBlendFactor srcFactor;
	VkBlendFactor dstFactor;
	VkBlendOp     op;
};

#define VKA_BLEND_OP_WRITE                                         \
	{                                                              \
		VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD \
	}
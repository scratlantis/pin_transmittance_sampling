#pragma once
#include <vka/resource_objects/resource_common.h>
typedef vka::Buffer_I    *VkaBuffer;
typedef vka::Image_I     *VkaImage;
typedef vka::CmdBuffer_I *VkaCommandBuffer;
typedef vka::AppState      *VkaState;

class DescriptorObjects; // sampler, image , buffer

namespace vka
{
	/*struct DrawCmd
	{
		VkaBuffer vertexBuffer;
		VkaBuffer indexBuffer;
	    std::vector<DescriptorObjects> descriptors;
		std::vector<VkaBuffer> instanceBuffers;
		RasterizationPipelineDefinition pipelineDef;
		RenderPassInstanceDefinition renderPassInstanceDef;
	};*/
	struct ImageSubresourceRange
    {
	    uint32_t mipLevel;
	    uint32_t baseArrayLayer;
	    uint32_t layerCount;
	};
    }		// namespace vka
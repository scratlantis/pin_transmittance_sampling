#pragma once
#include "Resource.h"
#include "PipelineLayout.h"
#include "Shader.h"

namespace vka
{

struct ComputePipelineState
{
	ShaderDefinition shaderDef;
	PipelineLayoutDefinition layoutDef;

	bool operator==(const ComputePipelineState& other) const
	{
		return shaderDef == other.shaderDef && layoutDef == other.layoutDef;
	}
	bool operator!=(const ComputePipelineState& other) const
	{
		return !(*this == other);
	}

	hash_t hash() const
	{
		hash_t hash = 0;
		hashCombine(hash, shaderDef.hash());
		hashCombine(hash, layoutDef.hash());
		return hash;
	};


	VkComputePipelineCreateInfo ComputePipelineState::buildPipelineCI(ResourceTracker *pTracker) const
	{
		VkComputePipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		pipelineCreateInfo.stage                                                                      = Shader(pTracker, shaderDef).getStageCI();
		pipelineCreateInfo.layout = PipelineLayout(pTracker, layoutDef).getHandle();
		pipelineCreateInfo.basePipelineHandle                                                         = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = -1;
		return pipelineCreateInfo;
	}
};

class ComputePipeline : public UniqueResource<VkPipeline>
{
  protected:
	void free()
	{
		vkDestroyPipeline(gState.device.logical, handle, nullptr);
	}
	void buildHandle()
	{
		VkComputePipelineCreateInfo ci = pipelineState.buildPipelineCI(pTracker);
		ASSERT_VULKAN(vkCreateComputePipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
	}
	virtual bool _equals(ComputePipeline const &other) const
	{
		return this->pipelineState == other.pipelineState;
	}
	ComputePipeline *copyToHeap() const
	{
		return new ComputePipeline(*this);
	}
  public:
	hash_t _hash() const
	{
		return pipelineState.hash();
	};
	ComputePipeline(ResourceTracker *pTracker, const ComputePipelineState pipelineState);
	~ComputePipeline();

	const ComputePipelineState       pipelineState;
  private:
};
}		// namespace vka
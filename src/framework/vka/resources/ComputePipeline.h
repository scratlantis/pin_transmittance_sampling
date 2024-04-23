#pragma once
#include "Resource.h"
#include "PipelineLayout.h"
#include "Shader.h"

namespace vka
{

struct ComputePipelineState
{
	ShaderDefinition shaderDef;
	PipelineLayoutDefinition pipelineLayoutDef;

	std::vector<uint32_t> specialisationEntrySizes;
	std::vector<uint8_t> specializationData;

	bool operator==(const ComputePipelineState& other) const
	{
		return shaderDef == other.shaderDef && pipelineLayoutDef == other.pipelineLayoutDef;
	}
	bool operator!=(const ComputePipelineState& other) const
	{
		return !(*this == other);
	}

	hash_t hash() const
	{
		hash_t hash = 0;
		hashCombine(hash, shaderDef.hash());
		hashCombine(hash, pipelineLayoutDef.hash());
		return hash;
	};


	VkComputePipelineCreateInfo ComputePipelineState::buildPipelineCI(ResourceTracker *pTracker) const
	{
		VkComputePipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo = Shader(pTracker, shaderDef).getStageCI();
		VkSpecializationInfo specializationInfo{};
		uint32_t                        specDataSize = writeSpecializationInfo(specialisationEntrySizes.data(), specialisationEntrySizes.size(), specializationData.data(), specializationInfo);
		if (specDataSize)
		{
			shaderStageCreateInfo.pSpecializationInfo = &specializationInfo;
		}
		pipelineCreateInfo.stage                                                                  = shaderStageCreateInfo;
		pipelineCreateInfo.layout = PipelineLayout(pTracker, pipelineLayoutDef).getHandle();
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
#pragma once
#include "Resource.h"
#include "PipelineLayout.h"
#include "Shader.h"

namespace vka
{

struct ComputePipelineState
{
  private:
	std::vector<VkSpecializationMapEntry> specMapEntries;
	VkSpecializationInfo specInfo;
  public:
	ShaderDefinition shaderDef;
	PipelineLayoutDefinition pipelineLayoutDef;

	std::vector<uint32_t> specialisationEntrySizes;
	std::vector<uint8_t> specializationData;

	bool operator==(const ComputePipelineState& other) const
	{
		bool isEqual = shaderDef == other.shaderDef && pipelineLayoutDef == other.pipelineLayoutDef;
		return isEqual;
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


	VkComputePipelineCreateInfo ComputePipelineState::buildPipelineCI(ResourceTracker *pTracker)
	{
		VkComputePipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo = Shader(pTracker, shaderDef).getStageCI();

		writeSpecializationInfo(specialisationEntrySizes, specializationData.data(), specMapEntries,specInfo);

		if (!specializationData.empty())
		{
			shaderStageCreateInfo.pSpecializationInfo = &specInfo;
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
		VK_CHECK(vkCreateComputePipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
	}
	//virtual bool _equals(ComputePipeline const &other) const
	//{
	//	return this->pipelineState == other.pipelineState;
	//}

	virtual bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		auto that = static_cast<ComputePipeline const &>(other);
		return *this == that;
	}



	ComputePipeline *copyToHeap() const
	{
		return new ComputePipeline(*this);
	}
  public:
	bool operator==(const ComputePipeline &other) const
	{
		bool isEqual = this->pipelineState == other.pipelineState;
		return isEqual;
		/*return this->pipelineState == other.pipelineState;*/
	}
	hash_t _hash() const
	{
		return pipelineState.hash();
	};
	ComputePipeline(ResourceTracker *pTracker, const ComputePipelineState pipelineState);
	~ComputePipeline();

	ComputePipelineState getState() const
	{
		return pipelineState;
	}
  private:
	ComputePipelineState       pipelineState;
};
}		// namespace vka
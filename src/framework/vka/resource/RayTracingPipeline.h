#pragma once
#include "PipelineLayout.h"
#include "Resource.h"
#include "Shader.h"
namespace vka
{


template<class T>
struct sbtRecord
{
	uint32_t      hitGroupID;
	T			  surfaceParams;
};

struct HitGroup
{
	uint32_t closestHitShaderID;
	uint32_t anyHitShaderID;

	bool operator==(const HitGroup &other) const
	{
		return (closestHitShaderID == other.closestHitShaderID) && (anyHitShaderID == other.anyHitShaderID);
	}
};



struct RayTracingPipelineState
{
	ShaderDefinition                                 rayGenShaderDef;
	ShaderDefinition                                 primaryMissShaderDef;
	ShaderDefinition                                 occlusionMissShaderDef;
	std::vector<ShaderDefinition>                    hitShaderDef;
	std::vector<HitGroup>                            hitGroups;
	std::vector<VkDynamicState>                      dynamicStates;
	PipelineLayoutDefinition                         layout;

	bool _equals(RayTracingPipelineState const &other) const;

	bool operator==(const RayTracingPipelineState &other) const
	{
		return _equals(other);
	}

	hash_t hash() const;

	VkRayTracingPipelineCreateInfoKHR buildPipelineCI(ResourceTracker *pTracker) const;

	RayTracingPipelineState();


};

class RayTracingPipeline : public UniqueResource<VkPipeline>
{
  protected:
	void free()
	{
		vkDestroyPipeline(gState.device.logical, handle, nullptr);
	}
	void buildHandle()
	{
		VkGraphicsPipelineCreateInfo ci = pipelineState.buildPipelineCI(pTracker);
		ASSERT_VULKAN(vkCreateGraphicsPipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
	}
	virtual bool _equals(RayTracingPipeline const &other) const
	{
		return this->pipelineState == other.pipelineState;
	}

	RayTracingPipeline *copyToHeap() const
	{
		return new RayTracingPipeline(pTracker, pipelineState);
	}

  public:
	hash_t _hash() const
	{
		hash_t hash = 0;
		hashCombine(hash, pipelineState.hash());
		return hash;
	};
	RayTracingPipeline(ResourceTracker *pTrackerconst, const RayTracingPipelineState pipelineState);
	~RayTracingPipeline();

	const RayTracingPipelineState    pipelineState;

  private:
};

}        // namespace vka
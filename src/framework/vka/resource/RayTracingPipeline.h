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
	ShaderDefinition                                 rayGenShader;
	ShaderDefinition                                 primaryMissShader;
	ShaderDefinition                                 occlusionMissShader;
	std::vector<VkDynamicState>                      dynamicStates;
	uint32_t                                         maxRecursionLevel;
	PipelineLayoutDefinition						 layoutDef;
	std::vector<ShaderDefinition>                  hitShaders;
	std::vector<HitGroup>                          hitGroups;
	VkRayTracingPipelineCreateInfoKHR                RayTracingPipelineState::buildPipelineCI(ResourceTracker *pTracker) const;

	



	bool _equals(RayTracingPipelineState const &other) const;
	bool operator==(const RayTracingPipelineState& other) const
	{
		return _equals(other);
	}
	hash_t hash() const;
	RayTracingPipelineState();


};

class RayTracingPipeline : public UniqueResource<VkPipeline>
{
  protected:
	void free()
	{
		vkDestroyPipeline(gState.device.logical, handle, nullptr);
	}
	void         buildHandle()
	{
		VkRayTracingPipelineCreateInfoKHR ci = pipelineState.buildPipelineCI(pTracker);
		ASSERT_VULKAN(vkCreateRayTracingPipelinesKHR(gState.device.logical, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
	}


	RayTracingPipeline *copyToHeap() const
	{
		return new RayTracingPipeline(*this);
	}

  public:
	bool _equals(const RayTracingPipeline& other) const
	{
		return pipelineState == other.pipelineState;
	}
	hash_t _hash() const
	{
		return pipelineState.hash();
	}
	RayTracingPipeline(ResourceTracker *pTracker, const RayTracingPipelineState pipelineState);
	~RayTracingPipeline();
  private:
	const RayTracingPipelineState pipelineState;
};


class RayTracingPipelineBuilder
{
  public:
	RayTracingPipelineBuilder();
	~RayTracingPipelineBuilder();

	VkPipeline getPipeline(ResourceTracker *pCache, ResourceTracker* pGarbage)
	{
		if (hasChanged)
		{
			RayTracingPipeline(pCache, lastState).move(pGarbage);
			return RayTracingPipeline(pCache, currentState).getHandle();
			lastState = currentState;
			hasChanged = false;
		}
		else
		{
			return RayTracingPipeline(pCache, currentState).getHandle();
		}
	}

	void newState(const ShaderDefinition rayGenShader, ShaderDefinition primaryMissShader, ShaderDefinition occlusionMissShader,
		std::vector<VkDynamicState> dynamicStates, uint32_t maxRecursionLevel, PipelineLayoutDefinition    layoutDef)
	{
		currentState.rayGenShader = rayGenShader;
		currentState.primaryMissShader = primaryMissShader;
		currentState.occlusionMissShader = occlusionMissShader;
		currentState.dynamicStates = dynamicStates;
		currentState.maxRecursionLevel = maxRecursionLevel;
		currentState.layoutDef = layoutDef;
		hasChanged = true;

	}

	void addHitShader(ShaderDefinition shader)
	{
		if (hitShaderMap.find(shader) == hitShaderMap.end())
		{
			hitShaderMap[shader] = currentState.hitShaders.size();
			currentState.hitShaders.push_back(shader);
			hasChanged = true;
		}
	}

	void addHitGroup(HitGroup group)
	{
		if (hitGroupMap.find(group) == hitGroupMap.end())
		{
			hitGroupMap[group] = currentState.hitGroups.size();
			currentState.hitGroups.push_back(group);
			hasChanged = true;
		}
	}

  private:
	bool hasChanged = false;
	std::unordered_map<ShaderDefinition, uint32_t> hitShaderMap;
	std::unordered_map<HitGroup, uint32_t>         hitGroupMap;
	RayTracingPipelineState currentState;
	RayTracingPipelineState lastState;
};



}        // namespace vka
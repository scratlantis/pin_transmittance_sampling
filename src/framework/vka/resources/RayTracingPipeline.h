#pragma once
#include "PipelineLayout.h"
#include "Resource.h"
#include "Shader.h"
namespace vka
{

template <class T>
struct sbtRecord
{
	uint32_t hitGroupID;
	T        surfaceParams;
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
}		// namespace vka
namespace std
{
template <>
struct hash<vka::HitGroup>
{
	size_t operator()(vka::HitGroup const &group) const
	{
		size_t h1 = hash<uint32_t>()(group.closestHitShaderID);
		size_t h2 = hash<uint32_t>()(group.anyHitShaderID);

		return ((h2 << 1) ^ h1);
	}
};
}        // namespace std
namespace vka
{
struct RayTracingPipelineState
{
	ShaderDefinition                                 rayGenShader;
	ShaderDefinition                                 primaryMissShader;
	ShaderDefinition                                 occlusionMissShader;
	std::vector<VkDynamicState>                      dynamicStates;
	uint32_t                                         maxRecursionLevel;
	PipelineLayoutDefinition						 layoutDef;
	std::vector<ShaderDefinition>					 hitShaders;
	std::vector<HitGroup>							 hitGroups;
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

	virtual bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		auto that = static_cast<RayTracingPipeline const &>(other);
		return *this == that;
	}
  public:
	//bool _equals(const RayTracingPipeline& other) const
	//{
	//	return pipelineState == other.pipelineState;
	//}

	bool operator==(const RayTracingPipeline &other) const
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
			lastState = currentState;
			hasChanged = false;
			return RayTracingPipeline(pCache, currentState).getHandle();
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

	uint32_t addHitShader(ShaderDefinition shader)
	{
		if (hitShaderMap.find(shader) == hitShaderMap.end())
		{
			hitShaderMap[shader] = currentState.hitShaders.size();
			currentState.hitShaders.push_back(shader);
			hasChanged = true;
			return currentState.hitShaders.size();
		}
		else
		{
			return hitShaderMap[shader];
		}
	}

	uint32_t addHitGroup(HitGroup group)
	{
		if (hitGroupMap.find(group) == hitGroupMap.end())
		{
			hitGroupMap[group] = currentState.hitGroups.size();
			currentState.hitGroups.push_back(group);
			hasChanged = true;
			return currentState.hitGroups.size();
		}
		else
		{
			return hitGroupMap[group];
		}
	}


	
void assignHitGroupIndices(ShaderDefinition closestHit, ShaderDefinition anyHit, ShaderDefinition occlusionAnyHit, uint32_t &hitGroupIdxPrimary, uint32_t &hitGroupIdxOcclusion)
	{
		if (closestHit == VKA_NULL_SHADER)
		{
			DEBUG_BREAK;
		}

		HitGroup hitGroupPrimary{};
		hitGroupPrimary.closestHitShaderID = addHitShader(closestHit);
		if (anyHit != VKA_NULL_SHADER)
		{
			hitGroupPrimary.anyHitShaderID = addHitShader(anyHit);
		}
		else
		{
			hitGroupPrimary.anyHitShaderID = VK_SHADER_UNUSED_KHR;
		}
		hitGroupIdxPrimary = addHitGroup(hitGroupPrimary);

		HitGroup hitGroupOcclusion{};
		hitGroupOcclusion.closestHitShaderID = VK_SHADER_UNUSED_KHR;
		if (occlusionAnyHit != VKA_NULL_SHADER)
		{
			hitGroupOcclusion.anyHitShaderID = addHitShader(occlusionAnyHit);
		}
		else
		{
			hitGroupOcclusion.anyHitShaderID = VK_SHADER_UNUSED_KHR;
		}
		hitGroupIdxOcclusion = addHitGroup(hitGroupOcclusion);
	}

  private:
	bool hasChanged = false;
	std::unordered_map<ShaderDefinition, uint32_t> hitShaderMap;
	std::unordered_map<HitGroup, uint32_t>         hitGroupMap;
	RayTracingPipelineState currentState;
	RayTracingPipelineState lastState;
};



}        // namespace vka
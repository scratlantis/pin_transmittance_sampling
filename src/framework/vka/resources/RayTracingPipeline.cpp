#include "RayTracingPipeline.h"
#include "../initializers/misc.h"
namespace vka
{




bool RayTracingPipelineState::_equals(RayTracingPipelineState const &other) const
{
	// clang-format off
	return rayGenShader == other.rayGenShader
		&& primaryMissShader == other.primaryMissShader
		&& occlusionMissShader == other.occlusionMissShader
		&& cmpArray(hitShaders, other.hitShaders)
		&& cmpArray(hitGroups, other.hitGroups)
		&& cmpArray(dynamicStates, other.dynamicStates)
		&& layoutDef == other.layoutDef
		&& maxRecursionLevel == other.maxRecursionLevel;
	// clang-format on
}

hash_t RayTracingPipelineState::hash() const
{
	hash_t hash = 0;
	hashCombine(hash, rayGenShader);
	hashCombine(hash, primaryMissShader);
	hashCombine(hash, occlusionMissShader);
	hashCombine(hash, hashArray(hitShaders));
	hashCombine(hash, shallowHashArray(hitGroups));
	hashCombine(hash, shallowHashArray(dynamicStates));
	hashCombine(hash, maxRecursionLevel);
	hashCombine(hash, layoutDef.hash());
	return hash;
}

VkRayTracingPipelineCreateInfoKHR RayTracingPipelineState::buildPipelineCI(ResourceTracker* pTracker)
{
	std::vector<VkPipelineShaderStageCreateInfo>      shaderStages;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

	{
		VkRayTracingShaderGroupCreateInfoKHR group = RayTracingShaderGroupCreateInfo_Empty();
		group.generalShader                        = 0;
		groups.push_back(group);
	}
	{
		VkRayTracingShaderGroupCreateInfoKHR group = RayTracingShaderGroupCreateInfo_Empty();
		group.generalShader                        = 1;
		groups.push_back(group);
	}

	{
		VkRayTracingShaderGroupCreateInfoKHR group = RayTracingShaderGroupCreateInfo_Empty();
		group.generalShader                        = 2;
		groups.push_back(group);
	}

	for (size_t i = 0; i < hitShaders.size(); i++)
	{
		shaderStages.push_back(Shader(pTracker, hitShaders[i]).getStageCI());
	}

	uint32_t hitShaderOffset = groups.size();
	for (size_t i = 0; i < hitGroups.size(); i++)
	{
		VkRayTracingShaderGroupCreateInfoKHR group = RayTracingShaderGroupCreateInfo_Empty();
		group.type                                 = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		if (hitGroups[i].closestHitShaderID != VK_SHADER_UNUSED_KHR)
		{
			group.closestHitShader = hitGroups[i].closestHitShaderID + hitShaderOffset;
		}
		if (hitGroups[i].anyHitShaderID != VK_SHADER_UNUSED_KHR)
		{
			group.anyHitShader = hitGroups[i].anyHitShaderID + hitShaderOffset;
		}
		groups.push_back(group);
	}

	VkPipelineLibraryCreateInfoKHR    pipelineLibraryCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR};
	VkPipelineDynamicStateCreateInfo  dynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	VkRayTracingPipelineCreateInfoKHR rayPipelineCreateInfo{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
	dynamicStateCreateInfo.dynamicStateCount           = dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates              = dynamicStates.data();
	rayPipelineCreateInfo.pDynamicState                = &dynamicStateCreateInfo;
	rayPipelineCreateInfo.stageCount                   = shaderStages.size();
	rayPipelineCreateInfo.pStages                      = shaderStages.data();
	rayPipelineCreateInfo.groupCount                   = groups.size();
	rayPipelineCreateInfo.pGroups                      = groups.data();
	rayPipelineCreateInfo.maxPipelineRayRecursionDepth = maxRecursionLevel;
	rayPipelineCreateInfo.pLibraryInfo                 = &pipelineLibraryCreateInfo;
	rayPipelineCreateInfo.layout                       = PipelineLayout(pTracker, layoutDef).getHandle();
	rayPipelineCreateInfo.basePipelineHandle           = VK_NULL_HANDLE;
	rayPipelineCreateInfo.basePipelineIndex            = -1;
	return rayPipelineCreateInfo;
}
RayTracingPipelineState::RayTracingPipelineState()
{
}
RayTracingPipeline::RayTracingPipeline(ResourceTracker *pTracker, const RayTracingPipelineState pipelineState) :
    UniqueResource(pTracker), pipelineState(pipelineState)
{
}

RayTracingPipeline::~RayTracingPipeline()
{
}

RayTracingPipelineBuilder::RayTracingPipelineBuilder()
{
}

RayTracingPipelineBuilder::~RayTracingPipelineBuilder()
{
}

}        // namespace vka
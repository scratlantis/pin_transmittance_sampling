#include "RayTracingPipeline.h"
#include "../initializers/misc.h"
namespace vka
{

VkGraphicsPipelineCreateInfo RayTracingPipelineState::buildPipelineCI(ResourceTracker *pTracker) const
{
	std::vector<VkPipelineShaderStageCreateInfo>      shaderStages;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

	{
		shaderStages.push_back(Shader(pTracker, rayGenShaderDef).getStageCI());
		VkRayTracingShaderGroupCreateInfoKHR group = RayTracingShaderGroupCreateInfo_Empty();
		group.generalShader                        = 0;
		groups.push_back(group);
	}
	{
		shaderStages.push_back(Shader(pTracker,primaryMissShaderDef).getStageCI());
		VkRayTracingShaderGroupCreateInfoKHR group = RayTracingShaderGroupCreateInfo_Empty();
		group.generalShader                        = 1;
		groups.push_back(group);
	}

	{
		shaderStages.push_back(Shader(pTracker, occlusionMissShaderDef).getStageCI());
		VkRayTracingShaderGroupCreateInfoKHR group = RayTracingShaderGroupCreateInfo_Empty();
		group.generalShader                        = 2;
		groups.push_back(group);
	}

	for (size_t i = 0; i < hitShaderDef.size(); i++)
	{
		shaderStages.push_back(Shader(pTracker, hitShaderDef[i]).getStageCI());
	}

	uint32_t hitShaderOffset = groups.size();
	for (size_t i = 0; i < hitGroups.size(); i++)
	{
		VkRayTracingShaderGroupCreateInfoKHR group = vk_default::emptyShaderGroupCI();
		group.type                                 = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		if (hitGroups[i].closestHitShader != VK_SHADER_UNUSED_KHR)
		{
			group.closestHitShader = hitGroups[i].closestHitShader + hitShaderOffset;
		}
		if (hitGroups[i].anyHitShader != VK_SHADER_UNUSED_KHR)
		{
			group.anyHitShader = hitGroups[i].anyHitShader + hitShaderOffset;
		}
		groups.push_back(group);
	}

	VkPipelineLibraryCreateInfoKHR    pipelineLibraryCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR};
	VkRayTracingPipelineCreateInfoKHR rayPipelineCreateInfo{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
	rayPipelineCreateInfo.stageCount                   = shaderStages.size();
	rayPipelineCreateInfo.pStages                      = shaderStages.data();
	rayPipelineCreateInfo.groupCount                   = groups.size();
	rayPipelineCreateInfo.pGroups                      = groups.data();
	rayPipelineCreateInfo.maxPipelineRayRecursionDepth = maxRecursionLevel;
	rayPipelineCreateInfo.pLibraryInfo                 = &pipelineLibraryCreateInfo;
	rayPipelineCreateInfo.layout                       = getPipelineLayout(pipeline_layout_definition_path_tracing_new);
	rayPipelineCreateInfo.basePipelineHandle           = VK_NULL_HANDLE;
	rayPipelineCreateInfo.basePipelineIndex            = -1;

	device->createRayTracingPipeline(&rayPipelineCreateInfo, &rayTracingPipeline);

	return pipelineCreateInfo;
}
}        // namespace vka
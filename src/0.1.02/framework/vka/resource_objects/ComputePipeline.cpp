#include "ComputePipeline.h"
#include "Shader.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
// Overrides start
hash_t ComputePipelineDefinition::hash() const
{
	hash_t hash = 0;
	hashCombineLocal(hash, shaderDef.hash());
	hashCombineLocal(hash, pipelineLayoutDefinition.hash());
	return hash;
}


DEFINE_EQUALS_OVERLOAD(ComputePipelineDefinition, ResourceIdentifier)

bool ComputePipelineDefinition::operator==(const ComputePipelineDefinition &other) const
{
	return shaderDef ==  other.shaderDef
		&& pipelineLayoutDefinition == other.pipelineLayoutDefinition;
}

void ComputePipeline::free()
{
	vkDestroyPipeline(gState.device.logical, handle, nullptr);
}

ComputePipeline::ComputePipeline(IResourceCache *pCache, ComputePipelineDefinition const &definition) :
    Cachable_T<VkPipeline>(pCache)
{
	VkComputePipelineCreateInfo     ci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = makeShaderStageCI(definition.shaderDef, pCache->fetch(definition.shaderDef));
	// Not pretty start
	ComputePipelineDefinition defCopy = definition;
	writeSpecializationInfo(
		defCopy.specialisationEntrySizes,
		defCopy.specializationData.data(),
		defCopy.specMapEntries,
		defCopy.specInfo);

	if (!defCopy.specializationData.empty())
	{
		shaderStageCreateInfo.pSpecializationInfo = &defCopy.specInfo;
	}
	// Not pretty end
	ci.stage              = shaderStageCreateInfo;
	ci.layout             = pCache->fetch(definition.pipelineLayoutDefinition);
	ci.basePipelineHandle = VK_NULL_HANDLE;
	ci.basePipelineIndex  = -1;
	VK_CHECK(vkCreateComputePipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
}


}        // namespace vka
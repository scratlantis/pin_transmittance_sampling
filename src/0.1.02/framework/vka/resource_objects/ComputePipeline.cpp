#include "ComputePipeline.h"
#include "Shader.h"
namespace vka
{
// Overrides start
hash_t ComputePipelineDefinition::hash() const
{
	hash_t hash = 0;
	hashCombine(hash, shaderDef.hash());
	hashCombine(hash, pipelineLayoutDef.hash());
	return hash;
}


bool ComputePipelineDefinition::_equals(ResourceIdentifier const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<ComputePipelineDefinition const &>(other);
		return equals(other_);
	}
}

bool ComputePipelineDefinition::equals(ComputePipelineDefinition const &other) const
{
	return shaderDef.equals(other.shaderDef) &&
	       pipelineLayoutDef.equals(other.pipelineLayoutDef);
}

hash_t ComputePipeline::hash() const
{
	return (hash_t) handle;
}

bool ComputePipeline::_equals(Resource const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<ComputePipeline const &>(other);
		return this->handle == other_.handle;
	}
}
// Overrides end

VkPipeline ComputePipeline::getHandle() const
{
	return handle;
}

void ComputePipeline::free()
{
	vkDestroyPipeline(gState.device.logical, handle, nullptr);
}

ComputePipeline::ComputePipeline(ComputePipelineDefinition const &definition)
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
	ci.layout             = pCache->fetch(definition.pipelineLayoutDef)->getHandle();
	ci.basePipelineHandle = VK_NULL_HANDLE;
	ci.basePipelineIndex  = -1;

	VK_CHECK(vkCreateComputePipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
}


}        // namespace vka
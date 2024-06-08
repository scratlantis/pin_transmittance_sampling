#include "PipelineLayout.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
hash_t PipelineLayoutDefinition::hash() const
{
	hash_t hash = byteHashVector(pcRanges);
	hashCombineLocal(hash, hashVector(descSetLayoutDef));
	return hash;
}
DEFINE_EQUALS_OVERLOAD(PipelineLayoutDefinition, ResourceIdentifier)
bool PipelineLayoutDefinition::operator==(const PipelineLayoutDefinition &other) const
{
	return memcmpVector(pcRanges, other.pcRanges) && cmpVector(descSetLayoutDef, other.descSetLayoutDef);
}

void PipelineLayout::free()
{
	vkDestroyPipelineLayout(gState.device.logical, handle, nullptr);
}
PipelineLayout::PipelineLayout(PipelineLayoutDefinition const &definition)
{
	VkPipelineLayoutCreateInfo         ci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	std::vector<VkDescriptorSetLayout> descSetLayouts;
	for (size_t i = 0; i < definition.descSetLayoutDef.size(); i++)
	{
		descSetLayouts.push_back(pCache->fetch(definition.descSetLayoutDef[i]));
	}
	ci.setLayoutCount         = descSetLayouts.size();
	ci.pSetLayouts            = descSetLayouts.data();
	ci.pushConstantRangeCount = definition.pcRanges.size();
	ci.pPushConstantRanges    = definition.pcRanges.data();
	VK_CHECK(vkCreatePipelineLayout(gState.device.logical, &ci, nullptr, &handle));
}

} // namespace vka
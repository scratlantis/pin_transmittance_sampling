#include "PipelineLayout.h"
namespace vka
{
hash_t PipelineLayoutDefinition::hash() const
{
	hash_t hash = shallowHashArray(pcRanges);
	hashCombine(hash, hashArray(descSetLayoutDef));
	return hash;
}

hash_t vka::PipelineLayout::hash() const
{
	return (hash_t) handle;
}

vka::PipelineLayout::PipelineLayout(PipelineLayoutDefinition const &definition)
{
	VkPipelineLayoutCreateInfo         ci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	std::vector<VkDescriptorSetLayout> descSetLayouts;
	for (size_t i = 0; i < definition.descSetLayoutDef.size(); i++)
	{
		descSetLayouts.push_back(pCache->fetch(definition.descSetLayoutDef[i])->getHandle());
	}
	ci.setLayoutCount         = descSetLayouts.size();
	ci.pSetLayouts            = descSetLayouts.data();
	ci.pushConstantRangeCount = definition.pcRanges.size();
	ci.pPushConstantRanges    = definition.pcRanges.data();
	VK_CHECK(vkCreatePipelineLayout(gState.device.logical, &ci, nullptr, &handle));
}

} // namespace vka
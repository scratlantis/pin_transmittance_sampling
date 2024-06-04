#include "PipelineLayout.h"
namespace vka
{
// Overrides start
hash_t PipelineLayoutDefinition::hash() const
{
	hash_t hash = shallowHashArray(pcRanges);
	hashCombine(hash, hashArray(descSetLayoutDef));
	return hash;
}

bool PipelineLayoutDefinition::_equals(ResourceIdentifier const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<PipelineLayoutDefinition const &>(other);
		return this->equals(other_);
	}
}
bool vka::PipelineLayoutDefinition::equals(PipelineLayoutDefinition const &other) const
{
	return shallowCmpArray(pcRanges, other.pcRanges) && cmpArray(descSetLayoutDef, other.descSetLayoutDef);
}



hash_t PipelineLayout::hash() const
{
	return (hash_t) handle;
}


bool PipelineLayout::_equals(Resource const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<PipelineLayout const &>(other);
		return this->handle == other_.handle;
	}
}
// Overrides end

VkPipelineLayout PipelineLayout::getHandle() const
{
	return VkPipelineLayout();
}

PipelineLayout::PipelineLayout(PipelineLayoutDefinition const &definition)
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
#include "DescriptorSetLayout.h"
namespace vka
{
hash_t DescriptorSetLayoutDefinition::hash() const
{
	hash_t hash = static_cast<hash_t>(flags);
	hashCombine(hash, shallowHashArray(bindings));
	return hash;
}

void DescriptorSetLayoutDefinition::addDescriptor(VkShaderStageFlags shaderStage, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding binding{};
	binding.binding            = VKA_COUNT(bindings);
	binding.descriptorType     = type;
	binding.descriptorCount    = 1;
	binding.stageFlags         = shaderStage;
	binding.pImmutableSamplers = nullptr;
	bindings.push_back(binding);
}



bool DescriptorSetLayout::_equals(Resource const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<DescriptorSetLayout const &>(other);
		return this->handle == other_.handle;
	}
}


DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayoutDefinition const &definition)
{
	VkDescriptorSetLayoutCreateInfo ci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	ci.flags        = definition.flags;
	ci.bindingCount = definition.bindings.size();
	ci.pBindings    = definition.bindings.data();
	VK_CHECK(vkCreateDescriptorSetLayout(gState.device.logical, &ci, nullptr, &handle));
}

void DescriptorSetLayout::free()
{
	vkDestroyDescriptorSetLayout(gState.device.logical, handle, nullptr);
}

hash_t DescriptorSetLayout::hash() const
{
	return (hash_t) (handle);
}


}        // namespace vka
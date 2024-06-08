#include "DescriptorSetLayout.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
hash_t DescriptorSetLayoutDefinition::hash() const
{
	return flags HASHC byteHashVector(bindings);
}

DEFINE_EQUALS_OVERLOAD(DescriptorSetLayoutDefinition, ResourceIdentifier)

bool DescriptorSetLayoutDefinition::operator==(const DescriptorSetLayoutDefinition &other) const
{
	return flags == other.flags && memcmpVector(bindings, other.bindings);
}



void DescriptorSetLayout::free()
{
	vkDestroyDescriptorSetLayout(gState.device.logical, handle, nullptr);
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayoutDefinition const &definition)
{
	VkDescriptorSetLayoutCreateInfo ci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	ci.flags        = definition.flags;
	ci.bindingCount = definition.bindings.size();
	ci.pBindings    = definition.bindings.data();
	VK_CHECK(vkCreateDescriptorSetLayout(gState.device.logical, &ci, nullptr, &handle));
}


void DescriptorSetLayoutDefinition::addDescriptor(VkShaderStageFlags shaderStage, VkDescriptorType type)
{
	ZERO_PAD(VkDescriptorSetLayoutBinding) binding{};
	binding.binding            = VKA_COUNT(bindings);
	binding.descriptorType     = type;
	binding.descriptorCount    = 1;
	binding.stageFlags         = shaderStage;
	binding.pImmutableSamplers = nullptr;
	bindings.push_back(binding);
}


}        // namespace vka
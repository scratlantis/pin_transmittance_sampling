#include "DescriptorSetLayout.h"
namespace vka
{
// Overrides start
hash_t DescriptorSetLayoutDefinition::hash() const
{
	hash_t hash = static_cast<hash_t>(flags);
	hashCombineLocal(hash, byteHashVector(bindings));
	return hash;
}

bool DescriptorSetLayoutDefinition::_equals(const ResourceIdentifier &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<DescriptorSetLayoutDefinition const &>(other);
		return equals(other_);
	}
}

bool DescriptorSetLayoutDefinition::equals(const DescriptorSetLayoutDefinition &other) const
{
	return flags == other.flags && memcmpVector(bindings, other.bindings);
}

hash_t DescriptorSetLayout::hash() const
{
	return (hash_t) (handle);
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
void DescriptorSetLayout::free()
{
	vkDestroyDescriptorSetLayout(gState.device.logical, handle, nullptr);
}
// Overrides end

VkDescriptorSetLayout DescriptorSetLayout::getHandle() const
{
	return handle;
}

void DescriptorSetLayout::free()
{
	vkDestroyDescriptorSetLayout(gState.device.logical, handle, nullptr);
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
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayoutDefinition const &definition)
{
	VkDescriptorSetLayoutCreateInfo ci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	ci.flags        = definition.flags;
	ci.bindingCount = definition.bindings.size();
	ci.pBindings    = definition.bindings.data();
	VK_CHECK(vkCreateDescriptorSetLayout(gState.device.logical, &ci, nullptr, &handle));
}


}        // namespace vka
#include "DescriptorSetLayout.h"

namespace vka
{
DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutDefinition &definition) :
    definition(definition)
{
	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}
}        // namespace vka
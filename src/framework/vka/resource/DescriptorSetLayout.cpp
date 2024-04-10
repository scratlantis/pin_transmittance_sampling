#include "DescriptorSetLayout.h"

namespace vka
{
DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutDefinition &definition, ResourceTracker* pTracker) :
    UniqueResource(pTracker), definition(definition)
{
	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}
}        // namespace vka
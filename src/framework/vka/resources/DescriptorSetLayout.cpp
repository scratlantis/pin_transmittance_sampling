#include "DescriptorSetLayout.h"

namespace vka
{
DescriptorSetLayout::DescriptorSetLayout(ResourceTracker *pTracker, const DescriptorSetLayoutDefinition &definition) :
    UniqueResource(pTracker), definition(definition)
{
	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}
}        // namespace vka
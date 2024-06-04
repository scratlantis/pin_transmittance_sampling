#include "ResourceCache.h"

#include <vka/resource_objects/DescriptorSetLayout.h>
namespace vka
{
DescriptorSetLayout* ResourceCache::fetch(const DescriptorSetLayoutDefinition &rID)
{
	DescriptorSetLayout *res = nullptr;
	auto it = descSetLayouts.find(rID);
	if (it != descSetLayouts.end())
		res = &it->second;
	else
	{
		res = new DescriptorSetLayout(rID);
	}
	return res;
}
}        // namespace vka


//void vka::ResourceCache::clear()
//{
//	auto it = resources.begin();
//	for (auto it = resources.begin(); it != resources.end(); ++it)
//	{
//		(*it)->free();
//		delete *it;
//	}
//	resources.clear();
//}
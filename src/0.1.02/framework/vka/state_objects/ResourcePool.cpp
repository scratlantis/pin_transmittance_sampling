#include "ResourcePool.h"
#include <vka/resource_objects/Resource.h>
namespace vka
{
bool ResourcePool::add(Resource *resource)
{
	return resources.insert(resource).second;
}
bool ResourcePool::remove(Resource *resource)
{
	return resources.erase(resource);
}

void ResourcePool::clear()
{
	auto it = resources.begin();
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	resources.clear();
}
}
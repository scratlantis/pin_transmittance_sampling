#include "ResourcePool.h"
#include <vka/resource_objects/Resource.h>
namespace vka
{
bool ResourcePool::add(Resource *resource)
{
	if (resource->hash() == 0)
	{
		printVka("ResourcePool::add() called with a resource that has a hash of 0\n");
		DEBUG_BREAK;
		return false;
	}

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
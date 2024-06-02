#include "Resource.h"
namespace vka
{
MappableResource::~MappableResource()
{
}

NonUniqueResource::~NonUniqueResource()
{
}

Resource *ResourceTracker::find(Resource *resource) const
{
	auto result = resources.find(resource);
	if (result != resources.end())
	{
		return *result;
	}
	else
	{
		return nullptr;
	}
}
bool ResourceTracker::add(Resource *resource)
{
	IF_VALIDATION(CHECK_TRUE(resource->pTracker != this))
	resource->pTracker = this;
	return resources.insert(resource).second;
}
bool ResourceTracker::remove(Resource *resource)
{
	IF_VALIDATION(CHECK_TRUE(resource->pTracker == this))
	resource->pTracker = nullptr;
	return resources.erase(resource);
}
bool ResourceTracker::move(Resource *resource, ResourceTracker *next)
{
	if (remove(resource))
	{
		return next->add(resource);
	}
	else
	{
		return false;
	}
}
void ResourceTracker::clear()
{
	auto it = resources.begin();
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	resources.clear();
}

}        // namespace vka
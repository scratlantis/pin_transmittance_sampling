#include "Resource.h"
namespace vka
{
MappableResource::~MappableResource()
{
}

NonUniqueResource::~NonUniqueResource()
{
}





Resource* ResourceTracker::find(Resource *resource)
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
void ResourceTracker::add(Resource *resource)
{
	resources.insert(resource);
}
void ResourceTracker::move(Resource *resource, ResourceTracker newTracker)
{
	Resource *r = find(resource);
	if (r)
	{
		newTracker.add(r);
	}
}
void ResourceTracker::clear()
{
	auto it = resources.begin();
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		(*it)->free();
	}
	resources.clear();
}








} // namespace vka
#pragma once
#include <unordered_set>
#include <vka/core/macros/misc.h>
#include <vka/core/functions/misc.h>
#include "Resource.h"
namespace vka
{
class ResourcePool
{
  private:
	std::unordered_set<Resource*> resources;

  public:
	bool add(Resource *resource);
	bool remove(Resource *resource);
	ResourcePool(){};
	~ResourcePool(){};
	void clear();
	DELETE_COPY_CONSTRUCTORS(ResourcePool);
};
}


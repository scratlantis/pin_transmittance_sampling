#pragma once
#include <unordered_set>
#include <vka/core/macros/misc.h>
#include <vka/core/functions/misc.h>
#include "Resource.h"
namespace vka
{
class ResourcePool : public IResourcePool
{
  private:
	std::unordered_set<Resource*> resources;

  public:
	bool add(Resource *resource) override;
	bool remove(Resource *resource) override;
	ResourcePool(){};
	~ResourcePool(){};
	void clear() override;
	DELETE_COPY_CONSTRUCTORS(ResourcePool);
};
}


#pragma once
#include <unordered_set>
#include <vka/core/common.h>
namespace vka
{
class Resource;
class ResourcePool
{
  private:
	std::unordered_set<Resource *, PointerObjHash<Resource>, PointerObjEq<Resource>> resources;

  public:
	bool add(Resource *resource);
	bool remove(Resource *resource);
	ResourcePool(){};
	~ResourcePool(){};
	void clear();
	DELETE_COPY_CONSTRUCTORS(ResourcePool);
};
}


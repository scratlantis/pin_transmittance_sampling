#pragma once
namespace vka
{
class Resource;
class IResourcePool
{
  public:
	virtual bool add(Resource *resource) = 0;
	virtual bool remove(Resource *resource) = 0;
	virtual void clear() = 0;
};
}        // namespace vka
#pragma once
#include <vka/core/common.h>
#include <unordered_set>
#include <vka/state_objects/ResourcePool.h>
#include <vka/state_objects/ResourceCache.h>

#define VKA_RESOURCE_META_DATA_HASH_OFFSET 0x1

namespace vka
{


class Resource
{
  protected:
	ResourcePool *pPool = nullptr;
	// interface
  protected:
	virtual bool _equals(Resource const &other) const = 0;

  public:
	virtual void   free() = 0;
	virtual void   track(ResourcePool *pPool);
	virtual void   garbageCollect();
	virtual hash_t hash() const = 0;

	Resource() :
	    pPool(nullptr){};
	~Resource(){};

	bool operator==(Resource const &other) const
	{
		std::string a = typeid(*this).name();
		std::string b = typeid(other).name();
		if (typeid(*this) != typeid(other))
		{
			return false;
		}
		else
		{
			return this->_equals(other);
		}
	}
};

class CachableResource : public Resource
{
  protected:
	ResourceCache *pCache = nullptr;
	// interface
  protected:
	virtual bool _equals(Resource const &other) const = 0;

  public:
	virtual void   free() = 0;
	virtual hash_t hash() const = 0;
	virtual void   track(ResourcePool *pPool);

	CachableResource() :
	    Resource(), pCache(nullptr){};
	~CachableResource(){};

	bool operator==(Resource const &other) const
	{
		std::string a = typeid(*this).name();
		std::string b = typeid(other).name();
		if (typeid(*this) != typeid(other))
		{
			return false;
		}
		else
		{
			return this->_equals(other);
		}
	}
};

class ResourceIdentifier
{
  protected:
	virtual bool _equals(ResourceIdentifier const &other) const = 0;

  public:
	ResourceIdentifier(){};
	~ResourceIdentifier(){};


	virtual hash_t hash() const = 0;


	bool operator==(ResourceIdentifier const &other) const
	{
		std::string a = typeid(*this).name();
		std::string b = typeid(other).name();
		if (typeid(*this) != typeid(other))
		{
			return false;
		}
		else
		{
			return this->_equals(other);
		}
	}
};

}        // namespace vka

namespace std
{
template <>
struct hash<vka::Resource>
{
	size_t operator()(vka::Resource const &r) const
	{
		return static_cast<size_t>(r.hash());
	}
};

template <>
struct hash<vka::Resource>
{
	size_t operator()(vka::ResourceIdentifier const &r) const
	{
		return static_cast<size_t>(r.hash());
	}
};
}        // namespace std
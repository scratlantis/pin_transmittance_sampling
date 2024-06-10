#pragma once
#include <vka/core/common.h>
#include <vka/state_objects/IResourceCache.h>
#include <vka/state_objects/IResourcePool.h>

//#include <vka/core/types/misc.h>

#include <unordered_set>

#define VKA_RESOURCE_META_DATA_HASH_SHIFT 0x1

namespace vka
{
class Resource
{
  protected:
	IResourcePool *pPool = nullptr;
  public:
	// required for unordered_set/unordered_map
	virtual bool   operator==(const Resource &other) const = 0;
	virtual hash_t hash() const = 0;

	virtual void   free() = 0;
	virtual void   track(IResourcePool *pPool);
	virtual void   garbageCollect();

	Resource() :
	    pPool(nullptr)
	{
	};
	~Resource(){};
};

template <typename T>
class Resource_T : public Resource
{
  protected:
	T handle;
  public:
	Resource_T(T handle) :
	    Resource(), handle(handle){};
	Resource_T() :
	    Resource(), handle(nullptr){};
	~Resource_T(){};
	hash_t hash() const override
	{
		return (hash_t) handle;
	}
	bool operator==(const Resource &other) const override
	{
		if (typeid(*this) == typeid(other))
		{
			auto &other_ = static_cast<Resource_T<T> const &>(other);
			return this->hash() == other_.hash();
		}
		return false;
	}
	virtual T getHandle() const
	{
		return handle;
	}
};

template <typename T>
class Cachable_T : public Resource
{
  protected:
	IResourceCache *pCache = nullptr;
	T               handle;
  public:
	Cachable_T() :
	    Resource(), pCache(nullptr){};
	~Cachable_T(){};


	hash_t hash() const override
	{
		return (hash_t) handle;
	}
	bool operator==(const Resource &other) const override
	{
		if (typeid(*this) == typeid(other))
		{
			auto &other_ = static_cast<Cachable_T<T> const &>(other);
			return this->hash() == other_.hash();
		}
		return false;
	}
	T            getHandle() const
	{
		return handle;
	}

	virtual void   track(IResourcePool* pPool) override
	{
		if (pCache == nullptr)
		{
			Resource::track(pPool);
		}
		else
		{
			printVka("CachableResource::track() called on a resource that is already cached\n");
			DEBUG_BREAK;
		}
	}
};

class ResourceIdentifier
{
  public:
	ResourceIdentifier(){};
	~ResourceIdentifier(){};

	virtual bool   operator==(const ResourceIdentifier &other) const = 0;
	virtual hash_t hash() const                                      = 0;
};
}        // namespace vka

//namespace std
//{
//template <>
//struct hash<vka::Resource>
//{
//	size_t operator()(vka::Resource const &r) const
//	{
//		return static_cast<size_t>(r.hash());
//	}
//};
//
//template <>
//struct hash<vka::ResourceIdentifier>
//{
//	size_t operator()(vka::ResourceIdentifier const &r) const
//	{
//		return static_cast<size_t>(r.hash());
//	}
//};   // namespace std
//}

DECLARE_HASH(vka::Resource, hash)
DECLARE_HASH(vka::ResourceIdentifier, hash)
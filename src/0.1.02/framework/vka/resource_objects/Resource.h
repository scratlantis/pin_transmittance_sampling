#pragma once
#include <vka/core/common.h>
#include <unordered_set>
#include <vka/state_objects/ResourcePool.h>
namespace vka
{


class Resource
{
  protected:
	ResourcePool  *pPool                                = nullptr;
	virtual bool   _equals(Resource const &other) const = 0;

  public:
	Resource():pPool(nullptr){};
	~Resource(){};
	virtual void free() = 0;
	virtual void  track(ResourcePool *pPool)
	{
		if (this->pPool)
		{
			if (this->pPool == pPool)
			{
				return;
			}

			if (this->pPool->remove(this))
			{
				this->pPool = pPool;
				this->pPool->add(this);
			}
			else
			{
				printVka("Resource not found in assigned pool\n");
				DEBUG_BREAK;
			}
		}
		else
		{
			this->pPool = pPool;
			this->pPool->add(this);
		}
	}
	virtual void garbageCollect()
	{
		track(&gState.frame->stack);
	}
	virtual hash_t hash() const                        = 0;

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
}

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
}        // namespace std
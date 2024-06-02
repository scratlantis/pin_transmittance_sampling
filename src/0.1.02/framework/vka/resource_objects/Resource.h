#include <vka/core/common.h>
#include <unordered_set>
namespace vka
{
class ResourceTracker
{
  public:
	ResourceTracker(){};
	~ResourceTracker(){};

	Resource *find(Resource *resource) const;
	void      clear();
	bool      move(Resource *resource, ResourceTracker *next);
	bool      add(Resource *resource);
	bool      remove(Resource *resource);

  private:
	std::unordered_set<Resource *, PointerObjHash<Resource>, PointerObjEq<Resource>> resources;
};

class Resource
{
  protected:
	ResourceTracker *pTracker;
	virtual void     free()                               = 0;
	virtual bool     _equals(Resource const &other) const = 0;

  public:
	virtual hash_t _hash() const = 0;
	friend class ResourceTracker;

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

	void move(ResourceTracker *pNewTracker)
	{
		Resource *result = pTracker->find(this);
		if (result)
		{
			pTracker->move(result, pNewTracker);
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
		return static_cast<size_t>(r._hash());
	}
};
}        // namespace std
#pragma once
#pragma once
#include "DescriptorSetLayout.h"
#include "Resource.h"


namespace vka
{
class Sampler : public UniqueResource<VkSampler>
{
  protected:
	void free()
	{
		vkDestroySampler(gState.device.logical, handle, nullptr);
	}
	void buildHandle()
	{
		VK_CHECK(vkCreateSampler(gState.device.logical, &ci, nullptr, &handle));
	}


	virtual bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		auto that = static_cast<Sampler const &>(other);
		return *this == that;
	}

	//virtual bool _equals(Sampler const &other) const
	//{
	//	return shallowCmpStructure(&this->ci, &other.ci);
	//}
	Sampler *copyToHeap() const
	{
		return new Sampler(pTracker, ci);
	}

  public:
	bool operator==(const Sampler &other) const
	{
		return shallowCmpStructure(&this->ci, &other.ci);
	}
	hash_t _hash() const
	{
		return shallowHashStructure(&ci);
	};
	Sampler(ResourceTracker *pTracker, const VkSamplerCreateInfo &ci);
	~Sampler();

	const VkSamplerCreateInfo ci;

  private:
};
}        // namespace vka

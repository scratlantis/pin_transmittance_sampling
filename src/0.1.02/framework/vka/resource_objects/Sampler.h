#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>

namespace vka
{
class SamplerDefinition : public ResourceIdentifier, public SamplerCreateInfo_Default
{
  public:
	hash_t hash() const;
	bool   _equals(ResourceIdentifier const &other) const override;
	bool   equals(SamplerDefinition const &other) const;

  protected:
};

class Sampler : public CachableResource
{
  private:
	VkSampler handle;

  protected:
  public:
	virtual bool     _equals(Resource const &other) const override;
	virtual hash_t   hash() const override;
	virtual void     free() override;
	VkSampler getHandle() const;
	Sampler(SamplerDefinition const &definition);
};
}        // namespace vka
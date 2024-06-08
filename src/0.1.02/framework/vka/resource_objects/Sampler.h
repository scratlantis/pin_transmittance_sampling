#pragma once
#include "Resource.h"

namespace vka
{
class SamplerDefinition : public ResourceIdentifier, public SamplerCreateInfo_Default
{
  public:
	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const SamplerDefinition &other) const;
	hash_t hash() const override;

  protected:
};

class Sampler : public Cachable_T<VkSampler>
{
  public:
	virtual void     free() override;
	Sampler(SamplerDefinition const &definition);
};
}        // namespace vka
DECLARE_HASH(vka::SamplerDefinition, hash)
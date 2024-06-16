#pragma once
#include "Resource.h"
#include "IDescriptor.h"
namespace vka
{
class SamplerDefinition : public ResourceIdentifier, public SamplerCreateInfo_Default, public IDescriptor
{
  public:
	SamplerDefinition() :
	    SamplerCreateInfo_Default(){};
	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const SamplerDefinition &other) const;
	hash_t hash() const override;

	void writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const override;
  protected:
};

class Sampler : public Cachable_T<VkSampler>
{
  public:
	virtual void     free() override;
	Sampler(IResourceCache *pCache, SamplerDefinition const &definition);


};
}        // namespace vka
DECLARE_HASH(vka::SamplerDefinition, hash)
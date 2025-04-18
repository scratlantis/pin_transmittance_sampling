#pragma once
#include "Resource.h"
#include "DescriptorSetLayout.h"

namespace vka
{
class PipelineLayoutDefinition : public ResourceIdentifier
{
  public:
	std::vector<VkPushConstantRange_OP>           pcRanges;
	std::vector<DescriptorSetLayoutDefinition> descSetLayoutDef;

	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const PipelineLayoutDefinition &other) const;
	hash_t hash() const override;
  protected:
};

class PipelineLayout : public Cachable_T <VkPipelineLayout>
{
  public:
	virtual void     free() override;
	PipelineLayout(IResourceCache *pCache, PipelineLayoutDefinition const &definition);
};
}        // namespace vka

DECLARE_HASH(vka::PipelineLayoutDefinition, hash)
#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>
#include "DescriptorSetLayout.h"

namespace vka
{
class PipelineLayoutDefinition : public ResourceIdentifier
{
  public:
	std::vector<VkPushConstantRange>           pcRanges;
	std::vector<DescriptorSetLayoutDefinition> descSetLayoutDef;

	hash_t hash() const;
	bool _equals(ResourceIdentifier const &other) const override;
	bool   equals(PipelineLayoutDefinition const &other) const;
  protected:
};

class PipelineLayout : public CachableResource
{
  private:
	VkPipelineLayout handle;
  protected:
  public:
	virtual bool     _equals(Resource const &other) const override;
	virtual hash_t   hash() const override;
	virtual void     free() override;
	VkPipelineLayout getHandle() const;
	PipelineLayout(PipelineLayoutDefinition const &definition);
};
}        // namespace vka
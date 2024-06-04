#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>
#include "DescriptorSetLayout.h"

namespace vka
{
struct PipelineLayoutDefinition : ResourceIdentifier
{
	std::vector<VkPushConstantRange>           pcRanges;
	std::vector<DescriptorSetLayoutDefinition> descSetLayoutDef;

	hash_t hash() const;
	bool operator==(const PipelineLayoutDefinition &other) const
	{
		bool isEqual = shallowCmpArray(pcRanges, other.pcRanges) && cmpArray(descSetLayoutDef, other.descSetLayoutDef);
		return isEqual;
	}
};
}        // namespace vka
namespace std
{
template <>
struct hash<vka::PipelineLayoutDefinition>
{
	size_t operator()(vka::PipelineLayoutDefinition const &r) const
	{
		return static_cast<size_t>(r.hash());
	}
};
}        // namespace std

namespace vka
{
class PipelineLayout : public CachableResource
{
  private:
	VkPipelineLayout handle;

  protected:
	virtual bool _equals(Resource const &other) const;

  public:
	PipelineLayout(PipelineLayoutDefinition const &definition);
	virtual void   free();
	virtual hash_t hash() const;
};
}        // namespace vka
#pragma once
#include "Resource.h"
#include "DescriptorSetLayout.h"

namespace vka
{

struct PipelineLayoutDefinition
{
	std::vector<VkPushConstantRange>           pcRanges;
	std::vector<DescriptorSetLayoutDefinition> descSetLayoutDef;

	hash_t hash() const
	{
		hash_t hash = shallowHashArray(pcRanges);
		hashCombine(hash, hashArray(descSetLayoutDef));
		return hash;
	}

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
class PipelineLayout : public UniqueResource<VkPipelineLayout>
{
  protected:
	void free()
	{
		vkDestroyPipelineLayout(gState.device.logical, handle, nullptr);
	}
	void buildHandle()
	{
		VkPipelineLayoutCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		std::vector<VkDescriptorSetLayout> descSetLayouts;
		for (size_t i = 0; i < definition.descSetLayoutDef.size(); i++)
		{
			descSetLayouts.push_back(DescriptorSetLayout(pTracker, definition.descSetLayoutDef[i]).getHandle());
		}
		ci.setLayoutCount         = descSetLayouts.size();
		ci.pSetLayouts            = descSetLayouts.data();
		ci.pushConstantRangeCount = definition.pcRanges.size();
		ci.pPushConstantRanges = definition.pcRanges.data();
		ASSERT_VULKAN(vkCreatePipelineLayout(gState.device.logical, &ci, nullptr, &handle));
		
	}
	//virtual bool _equals(PipelineLayout const &other) const
	//{
	//	return this->definition == other.definition;
	//}


	virtual bool _equals(Resource const &other) const
	{
		if (typeid(*this) != typeid(other))
			return false;
		auto that = static_cast<PipelineLayout const &>(other);
		return *this == that;
	}
	PipelineLayout *copyToHeap() const
	{
		return new PipelineLayout(*this);
	}

  public:
	bool operator==(const PipelineLayout &other) const
	{
		return this->definition == other.definition;
	}
	hash_t _hash() const
	{
		return definition.hash();
	};
	PipelineLayout(ResourceTracker *pTracker, const PipelineLayoutDefinition &definition);
	~PipelineLayout();

	const PipelineLayoutDefinition definition;

  private:
};
}        // namespace vka

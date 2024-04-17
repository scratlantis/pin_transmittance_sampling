#pragma once
#include "Resource.h"

namespace vka
{

struct DescriptorSetLayoutDefinition
{
	VkDescriptorSetLayoutCreateFlags          flags;
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	hash_t hash() const
	{
		hash_t hash = static_cast<hash_t>(flags);
		hashCombine(hash, shallowHashArray(bindings));
		return hash;
	}

	bool operator==(const DescriptorSetLayoutDefinition &other) const
	{
		return flags == other.flags && shallowCmpArray(bindings, other.bindings);
	}
};

}        // namespace vka

namespace std
{
template <>
struct hash<vka::DescriptorSetLayoutDefinition>
{
	size_t operator()(vka::DescriptorSetLayoutDefinition const &r) const
	{
		return static_cast<size_t>(r.hash());
	}
};
}        // namespace std



namespace vka
{
class DescriptorSetLayout : public UniqueResource <VkDescriptorSetLayout>
{
  protected:
	void free()
	{
		vkDestroyDescriptorSetLayout(gState.device.logical, handle, nullptr);
	}
	void buildHandle()
	{
		  VkDescriptorSetLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		  ci.flags = definition.flags;
		  ci.bindingCount = definition.bindings.size();
		  ci.pBindings = definition.bindings.data();
		  ASSERT_VULKAN(vkCreateDescriptorSetLayout(gState.device.logical, &ci, nullptr, &handle));
	}
	virtual bool _equals(DescriptorSetLayout const &other) const
	{
		return this->definition == other.definition;
	}

	DescriptorSetLayout* copyToHeap() const
	{
		return new DescriptorSetLayout(pTracker, definition);
	}
  public:


	hash_t _hash() const
	{
		return definition.hash();
	};
	DescriptorSetLayout(ResourceTracker *pTrackerconst, const DescriptorSetLayoutDefinition &definition);
	~DescriptorSetLayout();


	const DescriptorSetLayoutDefinition definition;

	
  private:

};
}



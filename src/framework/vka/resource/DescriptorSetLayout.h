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
		hashCombine(hash, hashArray((const uint8_t *) bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding)));
		return hash;
	}
};

class DescriptorSetLayout : public UniqueResource <VkDescriptorSetLayout>
{
  protected:
	void free()
	{
		vkDestroyDescriptorSetLayout(gState.device.logical, handle, nullptr);
	}
  public:
	hash_t _hash() const
	{
		return definition.hash();
	};
	DescriptorSetLayout(const DescriptorSetLayoutDefinition& definition);
	~DescriptorSetLayout();


	const DescriptorSetLayoutDefinition definition;

	
  private:
	  void buildHandle()
	  {
		  VkDescriptorSetLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		  ci.flags = definition.flags;
		  ci.bindingCount = definition.bindings.size();
		  ci.pBindings = definition.bindings.data();
		  ASSERT_VULKAN(vkCreateDescriptorSetLayout(gState.device.logical, &ci, nullptr, &handle));
	}

};
}



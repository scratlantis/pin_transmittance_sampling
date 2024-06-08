#pragma once
#include "Resource.h"

namespace vka
{
class DescriptorSetLayoutDefinition : public ResourceIdentifier
{
  public:
	VkDescriptorSetLayoutCreateFlags          flags;
	std::vector<ZERO_PAD(VkDescriptorSetLayoutBinding)> bindings;
	void   addDescriptor(VkShaderStageFlags shaderStage, VkDescriptorType type);

	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const DescriptorSetLayoutDefinition &other) const;
	hash_t hash() const override;
};

class DescriptorSetLayout : public Resource_T<VkDescriptorSetLayout>
{
  public:
	virtual void   free() override;
	DescriptorSetLayout(DescriptorSetLayoutDefinition const &definition);
};
}        // namespace vka
DECLARE_HASH(vka::DescriptorSetLayoutDefinition, hash)
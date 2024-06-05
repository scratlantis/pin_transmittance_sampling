#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>

namespace vka
{
class DescriptorSetLayoutDefinition : public ResourceIdentifier
{
  public:
	VkDescriptorSetLayoutCreateFlags          flags;
	std::vector<ZERO_PAD(VkDescriptorSetLayoutBinding)> bindings;
	void   addDescriptor(VkShaderStageFlags shaderStage, VkDescriptorType type);

	hash_t hash() const override;
	bool _equals(const ResourceIdentifier &other) const override;
	bool  equals(const DescriptorSetLayoutDefinition &other) const;
};

class DescriptorSetLayout : public Resource
{
  private:
	VkDescriptorSetLayout handle;
  public:
	virtual bool _equals(Resource const &other) const override;
	virtual hash_t        hash() const override;
	virtual void   free() override;
	VkDescriptorSetLayout getHandle() const;
	DescriptorSetLayout(DescriptorSetLayoutDefinition const &definition);
};
}        // namespace vka
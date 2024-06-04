#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>

namespace vka
{
class DescriptorSetLayoutDefinition : ResourceIdentifier
{
  public:
	VkDescriptorSetLayoutCreateFlags          flags;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	hash_t hash() const;
	void   addDescriptor(VkShaderStageFlags shaderStage, VkDescriptorType type);

  private:
	bool   operator==(const DescriptorSetLayoutDefinition &other) const
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
class DescriptorSetLayout : public Resource
{
  private:
	VkDescriptorSetLayout handle;
  protected:
	virtual bool _equals(Resource const &other) const;
  public:
	DescriptorSetLayout(DescriptorSetLayoutDefinition const &definition);
	virtual void   free();
	virtual hash_t hash() const;
	VkDescriptorSetLayout getHandle() const;
};
}        // namespace vka
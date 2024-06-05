#pragma once
#include "Resource.h"
#include <vka/state_objects/global_state.h>



namespace vka
{
class RenderPassDefinition : public ResourceIdentifier
{
  public:
	std::vector<ZERO_PAD(VkAttachmentDescription)> attachmentDescriptions;
	std::vector<SubpassDescription_OP>             subpassDescriptions;
	std::vector<ZERO_PAD(VkSubpassDependency)> subpassDependencies;

	std::vector<ZERO_PAD(VkAttachmentReference)> attachmentReferences;
	std::vector<uint32_t>              preserveAttachments;
	hash_t hash() const;
	bool   _equals(ResourceIdentifier const &other) const override;
	bool   equals(RenderPassDefinition const &other) const;
  protected:
};

class RenderPass : public CachableResource
{
  private:
	VkRenderPass handle;

  protected:
  public:
	virtual bool     _equals(Resource const &other) const override;
	virtual hash_t   hash() const override;
	virtual void     free() override;
	VkRenderPass       getHandle() const;
	RenderPass(RenderPassDefinition const &def);
};
}        // namespace vka
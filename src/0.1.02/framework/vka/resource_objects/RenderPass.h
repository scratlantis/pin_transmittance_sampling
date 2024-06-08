#pragma once
#include "Resource.h"



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
	bool                                         operator==(const ResourceIdentifier &other) const override;
	bool                                         operator==(const RenderPassDefinition &other) const;
	hash_t                                       hash() const override;
  protected:
};

class RenderPass : public Cachable_T<VkRenderPass>
{
  public:
	virtual void     free() override;
	RenderPass(RenderPassDefinition const &def);
};
}        // namespace vka
DECLARE_HASH(vka::RenderPassDefinition, hash)
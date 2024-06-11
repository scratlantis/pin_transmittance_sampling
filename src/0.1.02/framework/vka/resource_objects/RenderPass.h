#pragma once
#include "Resource.h"



namespace vka
{
//class Image_I;
//struct RenderPassInstanceDefinition
//{
//	Rect2D<float>           relViewport;
//	std::vector<Image_I *>  attachments;
//	std::vector<ClearValue> clearValues;
//	bool                      operator==(const RenderPassInstanceDefinition& other) const
//	{
//		return relViewport == other.relViewport
//			&& cmpVector(attachments, other.attachments)
//			&& cmpVector(clearValues, other.clearValues);
//	}
//	bool operator!=(const RenderPassInstanceDefinition& other) const
//	{
//		return !(*this == other);
//	}
//	VkRect2D getRenderArea() const
//	{
//		VkRect2D renderArea{};
//		VkExtent3D attachmentSize = attachments[0]->getExtent();
//		renderArea.offset         = {static_cast<int32_t>(relViewport.x * attachmentSize.width), static_cast<int>(relViewport.y *attachmentSize.height)};
//		renderArea.extent         = {static_cast<uint32_t>(relViewport.width * attachmentSize.width), static_cast<uint32_t>(relViewport.height *attachmentSize.height)};
//		return renderArea;
//	}
//
//	std::vector<VkClearValue> getClearValues() const
//	{
//		std::vector<VkClearValue> clearValues;
//		for (size_t i = 0; i < this->clearValues.size(); i++)
//		{
//			clearValues.push_back(this->clearValues[i].value);
//		}
//		return clearValues;
//	}
//};	


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
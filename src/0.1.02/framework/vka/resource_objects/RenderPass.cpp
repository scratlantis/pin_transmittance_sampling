#include "RenderPass.h"
namespace vka
{
// Overrides start
hash_t RenderPassDefinition::hash() const
{
	// clang-format off
	return shallowHashArray(attachmentDescriptions)
	    HASHC hashArray(subpassDescriptions)
	    HASHC shallowHashArray(subpassDependencies);
	// clang-format on
}

bool RenderPassDefinition::_equals(ResourceIdentifier const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<RenderPassDefinition const &>(other);
		return this->equals(other_);
	}
}
bool RenderPassDefinition::equals(RenderPassDefinition const &other) const
{
	// clang-format off
	return shallowCmpArray(attachmentDescriptions, other.attachmentDescriptions)
		&& cmpArray(subpassDescriptions, other.subpassDescriptions)
	    && shallowCmpArray(subpassDependencies, other.subpassDependencies);
	// clang-format on
}

hash_t RenderPass::hash() const
{
	return (hash_t) handle;
}

bool RenderPass::_equals(Resource const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<RenderPass const &>(other);
		return this->handle == other_.handle;
	}
}
void RenderPass::free()
{
	vkDestroyRenderPass(gState.device.logical, handle, nullptr);
}
// Overrides end

VkRenderPass RenderPass::getHandle() const
{
	return handle;
}

RenderPass::RenderPass(RenderPassDefinition const &def)
{
	VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderPassCreateInfo.attachmentCount = VKA_COUNT(def.attachmentDescriptions);
	renderPassCreateInfo.pAttachments    = def.attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount    = VKA_COUNT(def.subpassDescriptions);
	renderPassCreateInfo.pSubpasses      = def.subpassDescriptions.data();
	renderPassCreateInfo.dependencyCount = VKA_COUNT(def.subpassDependencies);
	renderPassCreateInfo.pDependencies   = def.subpassDependencies.data();
	VK_CHECK(vkCreateRenderPass(gState.device.logical, &renderPassCreateInfo, nullptr, &handle));
}

}        // namespace vka
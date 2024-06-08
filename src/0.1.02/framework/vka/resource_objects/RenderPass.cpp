#include "RenderPass.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
hash_t RenderPassDefinition::hash() const
{
	// clang-format off
	return byteHashVector(attachmentDescriptions)
	    HASHC hashVector(subpassDescriptions)
	    HASHC byteHashVector(subpassDependencies);
	// clang-format on
}

DEFINE_EQUALS_OVERLOAD(RenderPassDefinition, ResourceIdentifier)

bool RenderPassDefinition::operator==(const RenderPassDefinition &other) const
{
	// clang-format off
	return memcmpVector(attachmentDescriptions, other.attachmentDescriptions)
		&& cmpVector(subpassDescriptions, other.subpassDescriptions)
	    && memcmpVector(subpassDependencies, other.subpassDependencies);
	// clang-format on
}

void RenderPass::free()
{
	vkDestroyRenderPass(gState.device.logical, handle, nullptr);
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
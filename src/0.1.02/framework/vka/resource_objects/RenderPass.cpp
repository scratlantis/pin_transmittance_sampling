#include "RenderPass.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
hash_t RenderPassDefinition::hash() const
{
	// clang-format off
	return hashVector(attachmentDescriptions)
	    HASHC hashVector(subpassDescriptions)
	    HASHC hashVector(subpassDependencies);
	// clang-format on
}

DEFINE_EQUALS_OVERLOAD(RenderPassDefinition, ResourceIdentifier)

bool RenderPassDefinition::operator==(const RenderPassDefinition &other) const
{
	// clang-format off
	return cmpVector(attachmentDescriptions, other.attachmentDescriptions)
		&& cmpVector(subpassDescriptions, other.subpassDescriptions)
	    && cmpVector(subpassDependencies, other.subpassDependencies);
	// clang-format on
}

void RenderPass::free()
{
	vkDestroyRenderPass(gState.device.logical, handle, nullptr);
}

RenderPass::RenderPass(IResourceCache *pCache, RenderPassDefinition const &def)
	: Cachable_T<VkRenderPass>(pCache)
{
	VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderPassCreateInfo.attachmentCount = VKA_COUNT(def.attachmentDescriptions);
	renderPassCreateInfo.pAttachments    = def.attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount    = VKA_COUNT(def.subpassDescriptions);
	std::vector<VkSubpassDescription> vkSubpassDescriptions(def.subpassDescriptions.size());
	for (size_t i = 0; i < vkSubpassDescriptions.size(); i++)
	{
		vkSubpassDescriptions[i] = def.subpassDescriptions[i].getVulkanStruct();
	}
	renderPassCreateInfo.pSubpasses      = vkSubpassDescriptions.data();
	renderPassCreateInfo.dependencyCount = VKA_COUNT(def.subpassDependencies);
	renderPassCreateInfo.pDependencies   = def.subpassDependencies.data();
	VK_CHECK(vkCreateRenderPass(gState.device.logical, &renderPassCreateInfo, nullptr, &handle));
}

}        // namespace vka
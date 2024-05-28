#include "ConfigurableRenderPass.h"

namespace vka
{

void ConfigurableRenderPass::destroy()
{
	if (!framebuffers.empty())
	{
		for (size_t i = 0; i < gState.io.imageCount; i++)
		{
			vkDestroyFramebuffer(gState.device.logical, framebuffers[i], nullptr);
		}
		framebuffers.clear();
	}
	if (renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(gState.device.logical, renderPass, nullptr);
		renderPass = VK_NULL_HANDLE;
	}
}

void ConfigurableRenderPass::createRenderPass()
{
	ASSERT_TRUE(renderPass == VK_NULL_HANDLE);

	VkAttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	VkAttachmentReference                colorAttachmentReference{};
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> colorAttachmentReferences;
	for (size_t i = 0; i < pColorAttachments.size(); i++)
	{
		colorAttachmentDescription.loadOp        = colorClear[i] ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachmentDescription.finalLayout   = colorTargetLayout[i];
		colorAttachmentDescription.initialLayout = colorInitialLayout[i];
		colorAttachmentDescription.format        = pColorAttachments[i]->format;
		attachments.push_back(colorAttachmentDescription);
		colorAttachmentReference.attachment = i;
		colorAttachmentReferences.push_back(colorAttachmentReference);
	}
	if (pDepthImage != nullptr)
	{
		VkAttachmentDescription depthAttachmentDescription{};
		depthAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
		depthAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentDescription.format         = VK_FORMAT_D32_SFLOAT;
		attachments.push_back(depthAttachmentDescription);
	}

	VkAttachmentReference depthAttachmetReference{};
	depthAttachmetReference.attachment = colorAttachmentReferences.size();
	depthAttachmetReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount    = colorAttachmentReferences.size();
	subpassDescription.pColorAttachments       = colorAttachmentReferences.data();
	if (pDepthImage != nullptr)
	{
		subpassDescription.pDepthStencilAttachment = &depthAttachmetReference;
	}

	std::vector<VkSubpassDependency> subpassDependencies;
	{
		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass      = 0;
		subpassDependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		subpassDependencies.push_back(subpassDependency);
	}
	{
		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass      = 0;
		subpassDependency.dstSubpass      = VK_SUBPASS_EXTERNAL;
		subpassDependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		subpassDependencies.push_back(subpassDependency);
	}

	

	

	VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments    = attachments.data();
	renderPassCreateInfo.subpassCount    = 1;
	renderPassCreateInfo.pSubpasses      = &subpassDescription;
	renderPassCreateInfo.dependencyCount = subpassDependencies.size();
	renderPassCreateInfo.pDependencies   = subpassDependencies.data();

	ASSERT_VULKAN(vkCreateRenderPass(gState.device.logical, &renderPassCreateInfo, nullptr, &renderPass));
}

void ConfigurableRenderPass::createFramebuffers()
{
	ASSERT_TRUE(renderPass != VK_NULL_HANDLE);
	ASSERT_TRUE(framebuffers.empty());
	framebuffers.resize(gState.io.imageCount);
	framebufferExtent = gState.io.extent;
	if (pDepthImage != nullptr)
	{
		framebufferExtent = {pDepthImage->extent.width, pDepthImage->extent.height};
	}
	if (!pColorAttachments.empty())
	{
		framebufferExtent = {pColorAttachments[0]->extent.width, pColorAttachments[0]->extent.height};
	}
	for (size_t i = 0; i < gState.io.imageCount; i++)
	{
		VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferCreateInfo.renderPass      = renderPass;
		std::vector<VkImageView> attachmentViews;
		for (size_t i = 0; i < pColorAttachments.size(); i++)
		{
			attachmentViews.push_back(pColorAttachments[i]->view);
		}
		if (pDepthImage != nullptr)
		{
			attachmentViews.push_back(pDepthImage->view);
		}
		framebufferCreateInfo.attachmentCount = attachmentViews.size();
		framebufferCreateInfo.pAttachments = attachmentViews.data();
		framebufferCreateInfo.width        = framebufferExtent.width;
		framebufferCreateInfo.height       = framebufferExtent.height;
		framebufferCreateInfo.layers       = 1;
		ASSERT_VULKAN(vkCreateFramebuffer(gState.device.logical, &framebufferCreateInfo, nullptr, &framebuffers[i]));
	}
}

void ConfigurableRenderPass::updatFramebuffers()
{
	if ((framebufferExtent.width == gState.io.extent.width && framebufferExtent.height == gState.io.extent.height) && !gState.swapchainRecreated())
	{
		return;
	}
	else
	{
		for (size_t i = 0; i < gState.io.imageCount; i++)
		{
			vkDestroyFramebuffer(gState.device.logical, framebuffers[i], nullptr);
		}
		framebuffers.clear();
		createFramebuffers();
	}
}

ConfigurableRenderPass::ConfigurableRenderPass(ConfigurableRenderPassCI ci):
    pDepthImage(ci.pDepthImage), pColorAttachments(ci.pColorAttachments),
	colorClearValues(ci.colorClearValues), colorClear(ci.colorClear), colorInitialLayout(ci.colorInitialLayout),
	colorTargetLayout(ci.colorTargetLayout), relRenderArea(ci.relRenderArea)
{
}
ConfigurableRenderPass::~ConfigurableRenderPass()
{
}

void ConfigurableRenderPass::init()
{
	createRenderPass();
	createFramebuffers();
}

void ConfigurableRenderPass::beginRender(UniversalCmdBuffer &cmdBuf)
{
	updatFramebuffers();
	std::vector<VkClearValue> clearValues = colorClearValues;
	if (pDepthImage != nullptr)
	{
		clearValues.push_back({1.0f, 0});
	}

	VkRect2D renderArea{};
	renderArea.offset = {static_cast<int32_t>(relRenderArea.x * gState.io.extent.width), static_cast<int>(relRenderArea.y * gState.io.extent.height)};
	renderArea.extent = {static_cast<uint32_t>(relRenderArea.width * gState.io.extent.width), static_cast<uint32_t>(relRenderArea.height * gState.io.extent.height)};
	cmdBuf.startRenderPass(renderPass, framebuffers[gState.frame->frameIndex], clearValues, renderArea);
}

void ConfigurableRenderPass::endRender(UniversalCmdBuffer &cmdBuf)
{
	for (size_t i = 0; i < pColorAttachments.size(); i++)
	{
		pColorAttachments[i]->layout = colorTargetLayout[i];
	}
	cmdBuf.endRenderPass();
}

RasterizationPipeline ConfigurableRenderPass::createPipeline(const RasterizationPipelineState pipelineState, uint32_t subpassIndex) const
{
	return RasterizationPipeline(&gState.cache, pipelineState, renderPass, subpassIndex);
}
}        // namespace vka
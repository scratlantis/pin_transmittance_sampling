#include "DefaultRenderPass.h"

namespace vka
{

void DefaultRenderPass::destroy()
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

void DefaultRenderPass::createRenderPass()
{
	ASSERT_TRUE(renderPass == VK_NULL_HANDLE);
	VkAttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	if (pOffscreenImage == nullptr)
	{
		colorAttachmentDescription.format = gState.io.format;
	}
	else
	{
		colorAttachmentDescription.format = pOffscreenImage->format;
	}
	colorAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachmentDescription{};
	depthAttachmentDescription.samples                   = VK_SAMPLE_COUNT_1_BIT;
	depthAttachmentDescription.loadOp                    = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachmentDescription.storeOp                   = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachmentDescription.stencilLoadOp             = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachmentDescription.stencilStoreOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.initialLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachmentDescription.finalLayout               = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachmentDescription.format                    = VK_FORMAT_D32_SFLOAT;


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

	VkAttachmentReference colorAttachmetReference{};
	colorAttachmetReference.attachment = 0;
	colorAttachmetReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmetReference{};
	depthAttachmetReference.attachment = 1;
	depthAttachmetReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments       = &colorAttachmetReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachmetReference;


	std::vector<VkAttachmentDescription> attachments = {colorAttachmentDescription, depthAttachmentDescription};

	VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderPassCreateInfo.attachmentCount = attachments.size();
	renderPassCreateInfo.pAttachments    = attachments.data();
	renderPassCreateInfo.subpassCount    = 1;
	renderPassCreateInfo.pSubpasses      = &subpassDescription;
	renderPassCreateInfo.dependencyCount = subpassDependencies.size();
	renderPassCreateInfo.pDependencies   = subpassDependencies.data();

	ASSERT_VULKAN(vkCreateRenderPass(gState.device.logical, &renderPassCreateInfo, nullptr, &renderPass));
}

void DefaultRenderPass::createFramebuffers()
{
	ASSERT_TRUE(renderPass != VK_NULL_HANDLE);
	ASSERT_TRUE(framebuffers.empty());
	framebuffers.resize(gState.io.imageCount);
	if (pOffscreenImage == nullptr)
	{
		framebufferExtent = gState.io.extent;
	}
	else
	{
		framebufferExtent = {pOffscreenImage->extent.width, pOffscreenImage->extent.height};
	}
	framebufferExtent = gState.io.extent;
	for (size_t i = 0; i < gState.io.imageCount; i++)
	{
		VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferCreateInfo.renderPass      = renderPass;
		framebufferCreateInfo.attachmentCount = 2;
		VkImageView attachments[2];
		if (pOffscreenImage == nullptr)
		{
			attachments[0] = gState.io.images[i].view;
		}
		else
		{
			attachments[0] = pOffscreenImage->view;
		}
		attachments[1] = pDepthImage->view;
		framebufferCreateInfo.pAttachments = &attachments[0];
		framebufferCreateInfo.width           = framebufferExtent.width;
		framebufferCreateInfo.height          = framebufferExtent.height;
		framebufferCreateInfo.layers          = 1;
		ASSERT_VULKAN(vkCreateFramebuffer(gState.device.logical, &framebufferCreateInfo, nullptr, &framebuffers[i]));
	}
}

void DefaultRenderPass::updatFramebuffers()
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

DefaultRenderPass::DefaultRenderPass(Image* pDepthImage, Image* pOffscreenImage) :
    pOffscreenImage(pOffscreenImage),
    pDepthImage(pDepthImage)
{
}
DefaultRenderPass::~DefaultRenderPass()
{
}

void DefaultRenderPass::init()
{
	createRenderPass();
	createFramebuffers();
}

void DefaultRenderPass::beginRender(UniversalCmdBuffer& cmdBuf)
{
	updatFramebuffers();
	std::vector<VkClearValue> clearValues = {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0}};
	cmdBuf.startRenderPass(renderPass, framebuffers[gState.frame->frameIndex], clearValues);
}

void DefaultRenderPass::endRender(UniversalCmdBuffer &cmdBuf)
{
	if (pOffscreenImage != nullptr)
	{
		pOffscreenImage->layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	cmdBuf.endRenderPass();

}

RasterizationPipeline DefaultRenderPass::createPipeline(const RasterizationPipelineState pipelineState, uint32_t subpassIndex) const
{
	return RasterizationPipeline(&gState.cache, pipelineState, renderPass, subpassIndex);
}
}
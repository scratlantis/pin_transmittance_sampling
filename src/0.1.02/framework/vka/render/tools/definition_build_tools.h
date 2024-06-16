#pragma once
#include <vka/interface/types.h>
#include <vka/resource_objects/resource_common.h>
#include <vka/render/tools/FramebufferCache.h>

inline VkSubpassDependency_OP initialSubpassDependency()
{
	VkSubpassDependency_OP dependency{};
	dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass      = 0;
	dependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependency;
}
inline VkSubpassDependency_OP finalSubpassDependency()
{
	VkSubpassDependency_OP dependency{};
	dependency.srcSubpass      = 0;
	dependency.dstSubpass      = VK_SUBPASS_EXTERNAL;
	dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependency;
}

inline void setDefaults(RenderPassDefinition &def)
{
	def = {};
	VkaSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	def.subpassDescriptions              = {subpassDescription};
	def.subpassDependencies              = {initialSubpassDependency(), finalSubpassDependency()};
}

struct AttachmentLayoutDescription
{
	VkImageLayout in;
	VkImageLayout internal;
	VkImageLayout out;
};

inline void addAttachment(RenderPassDefinition &def, AttachmentLayoutDescription layout, VkFormat format, bool clear)
{
	VkAttachmentDescription attachment{};
	attachment.format         = format;
	attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp         = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout  = layout.in;
	attachment.finalLayout    = layout.out;
	def.attachmentDescriptions.push_back(attachment);
	VkAttachmentReference_OP ref{};
	ref.attachment                = def.attachmentDescriptions.size() - 1;
	ref.layout                    = layout.internal;
	switch (layout.internal)
	{
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			def.subpassDescriptions[def.currentSubpass].colorAttachments.push_back(ref);
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			def.subpassDescriptions[def.currentSubpass].depthStencilAttachment = ref;
			break;
		default:
			printVka("Unsupported internal layout");
			DEBUG_BREAK;
			break;
	}
}
inline void addColorAttachment(RenderPassDefinition &def, VkImageLayout layoutIn, VkImageLayout layoutOut, VkFormat format, bool clear)
{
	addAttachment(def, {layoutIn, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, layoutOut}, format, clear);
}

inline void addDepthAttachment(RenderPassDefinition &def, VkFormat format, bool clear)
{
	addAttachment(def, {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}, format, clear);
}

// Rasterization Pipeline

inline void nextSubpass(RenderPassDefinition &def)
{
	auto &subpassDescription             = def.subpassDescriptions.emplace_back();
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	def.currentSubpass++;
}

inline void setDefaults(RasterizationPipelineDefinition &def, RasterizationPipelineInitValues &initValues)
{
	def                                       = {};
	def.flags                                 = initValues.flags;
	def.multisampleState.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	def.multisampleState.rasterizationSamples = initValues.sampleCount;
	def.multisampleState.minSampleShading     = initValues.minSampleShading;
	def.inputAssemblyState.sType              = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	def.inputAssemblyState.topology           = initValues.primitiveTopology;
	def.tessellationState.sType               = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	def.rasterizationState.sType              = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	def.rasterizationState.lineWidth          = initValues.lineWidth;
	def.depthStencilState.sType               = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	def.depthStencilState.minDepthBounds      = initValues.minDepthBounds;
	def.depthStencilState.maxDepthBounds      = initValues.maxDepthBounds;
	def.globalColorBlendState.logicOp         = VK_LOGIC_OP_NO_OP;
	def.dynamicStates                         = initValues.dynamicStates;
	setDefaults(def.renderPassDefinition);
}

inline void addInput(RasterizationPipelineDefinition &def, VertexDataLayout inputLayout, VkVertexInputRate inputRate)
{
	VKA_ASSERT(inputLayout.formats.size() == inputLayout.offsets.size());
	for (size_t i = 0; i < inputLayout.formats.size(); i++)
	{
		VkVertexInputAttributeDescription_OP attributeDesc{};
		attributeDesc.location = i;
		attributeDesc.binding  = def.vertexBindingDescriptions.size();
		attributeDesc.format   = inputLayout.formats[i];
		attributeDesc.offset   = inputLayout.offsets[i];
		def.vertexAttributeDescriptions.push_back(attributeDesc);
	}
	VkVertexInputBindingDescription_OP bindingDesc{};
	bindingDesc.binding   = def.vertexBindingDescriptions.size();
	bindingDesc.stride    = inputLayout.stride;
	bindingDesc.inputRate = inputRate;
	def.vertexBindingDescriptions.push_back(bindingDesc);
}

inline void addShader(RasterizationPipelineDefinition &def, std::string path, std::vector<ShaderArgs> args)
{
	def.shaderDefinitions.push_back(ShaderDefinition(path, args));
}

inline void addUnusedAttachmentState(RasterizationPipelineDefinition &def)
{
	def.colorBlendAttachmentStates.push_back({});
}

inline void addBlendAttachmentState(RasterizationPipelineDefinition &def, BlendOperation colorBlendOp, BlendOperation alphaBlendOp,
                                    VkColorComponentFlags colorWriteFlags = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
{
	VkPipelineColorBlendAttachmentState_OP attachmentBlendState{};
	attachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentBlendState.blendEnable    = VK_TRUE;

	attachmentBlendState.srcAlphaBlendFactor = colorBlendOp.srcFactor;
	attachmentBlendState.dstAlphaBlendFactor = colorBlendOp.dstFactor;
	attachmentBlendState.alphaBlendOp        = colorBlendOp.op;

	attachmentBlendState.srcColorBlendFactor = alphaBlendOp.srcFactor;
	attachmentBlendState.dstColorBlendFactor = alphaBlendOp.dstFactor;
	attachmentBlendState.colorBlendOp        = alphaBlendOp.op;
	def.colorBlendAttachmentStates.push_back(attachmentBlendState);
}

inline void addDepthAttachment(RasterizationPipelineDefinition &def, VkaImage depthImage, VkBool32 enableWrite, VkCompareOp compareOp, bool clear)
{
	if (enableWrite || compareOp != VK_COMPARE_OP_ALWAYS)
	{
		def.depthStencilState.depthTestEnable = VK_TRUE;
	}
	def.depthStencilState.depthWriteEnable = enableWrite;
	def.depthStencilState.depthCompareOp   = compareOp;
	addDepthAttachment(def.renderPassDefinition, depthImage->getFormat(), clear);
}

inline void addUnusedColorAttachment(RasterizationPipelineDefinition &def, VkaImage image,
                                     VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear)
{
	addUnusedAttachmentState(def);
	addColorAttachment(def.renderPassDefinition, layoutIn, layoutOut, image->getFormat(), clear);
}

inline void addColorAttachment(RasterizationPipelineDefinition &def, VkaImage image,
                               VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear,
                               BlendOperation colorBlendOp, BlendOperation alphaBlendOp)
{
	addBlendAttachmentState(def, colorBlendOp, alphaBlendOp);
	addColorAttachment(def.renderPassDefinition, layoutIn, layoutOut, image->getFormat(), clear);
}

inline void addDescriptor(RasterizationPipelineDefinition &def, VkShaderStageFlags shaderStage, VkDescriptorType type)
{
	if (def.pipelineLayoutDefinition.descSetLayoutDef.empty())
	{
		def.pipelineLayoutDefinition.descSetLayoutDef.push_back({});
	}
	def.pipelineLayoutDefinition.descSetLayoutDef.back().addDescriptor(shaderStage, type);
}

inline void nextDescriptorSet(RasterizationPipelineDefinition &def)
{
	def.pipelineLayoutDefinition.descSetLayoutDef.push_back({});
}

// Draw State
inline void addDepthAttachment(DrawCmd &drawCmd, VkaImage depthImage, bool clear, VkBool32 enableWrite, VkCompareOp compareOp)
{
	addDepthAttachment(drawCmd.pipelineDef, depthImage, enableWrite, compareOp, clear);
	drawCmd.attachments.push_back(depthImage);
	drawCmd.clearValues.push_back(VK_CLEAR_COLOR_MAX_DEPTH);
}

inline void addColorAttachment(DrawCmd &drawCmd, VkaImage image, ClearValue clearValue,
                               VkImageLayout layoutIn, VkImageLayout layoutOut,
                               BlendOperation colorBlendOp, BlendOperation alphaBlendOp)
{
	addColorAttachment(drawCmd.pipelineDef, image, layoutIn, layoutOut, clearValue.type != CLEAR_VALUE_NONE, colorBlendOp, alphaBlendOp);
	drawCmd.attachments.push_back(image);
	drawCmd.clearValues.push_back(clearValue);
}

inline void createFramebuffer(DrawCmd &drawCmd, FramebufferCache &framebufferCache)
{
	drawCmd.framebuffer = framebufferCache.fetch(gState.cache->fetch(drawCmd.pipelineDef.renderPassDefinition), drawCmd.attachments);
}

inline void addDescriptor(DrawCmd &drawCmd, IDescriptor *desc, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	addDescriptor(drawCmd.pipelineDef, shaderStage, type);
	drawCmd.descriptors.push_back(desc);
}
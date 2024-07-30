#pragma once
#include "complex_command_types.h"
namespace vka
{
struct AttachmentLayoutDescription
{
	VkImageLayout in;
	VkImageLayout internal;
	VkImageLayout out;
};

VkSubpassDependency_OP          initialSubpassDependency();
VkSubpassDependency_OP          finalSubpassDependency();
RenderPassDefinition            defaultRenderPass();
void                            addAttachment(RenderPassDefinition &def, AttachmentLayoutDescription layout, VkFormat format, bool clear);
void                            addColorAttachment(RenderPassDefinition &def, VkImageLayout layoutIn, VkImageLayout layoutOut, VkFormat format, bool clear);
void                            addDepthAttachment(RenderPassDefinition &def, VkFormat format, bool clear);
void                            nextSubpass(RenderPassDefinition &def);
RasterizationPipelineDefinition defaultRasterizationPipeline(RasterizationPipelineInitValues &initValues);
RasterizationPipelineDefinition defaultRasterizationPipeline();
void                            addInput(RasterizationPipelineDefinition &def, VertexDataLayout inputLayout, VkVertexInputRate inputRate);
void                            addShader(RasterizationPipelineDefinition &def, std::string path, std::vector<ShaderArgs> args);
void                            addWriteAttachmentState(RasterizationPipelineDefinition &def);
void                            addBlendAttachmentState(RasterizationPipelineDefinition &def, BlendOperation colorBlendOp, BlendOperation alphaBlendOp, VkColorComponentFlags colorWriteFlags);
void                            addDepthAttachment(RasterizationPipelineDefinition &def, Image depthImage, VkBool32 enableWrite, VkCompareOp compareOp, bool clear);
void                            addWriteColorAttachment(RasterizationPipelineDefinition &def, Image image, VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear);
void                            addBlendColorAttachment(RasterizationPipelineDefinition &def, Image image, VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear, BlendOperation colorBlendOp, BlendOperation alphaBlendOp);
void                            addDescriptor(RasterizationPipelineDefinition &def, VkShaderStageFlags shaderStage, VkDescriptorType type);
void                            nextDescriptorSet(RasterizationPipelineDefinition &def);
void                            addDescriptor(ComputePipelineDefinition &def, VkDescriptorType type);
}
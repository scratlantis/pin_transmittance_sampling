#include "complex_commands.h"
#include "complex_command_construction.h"
#include <vka/core/core_utility/buffer_utility.h>
#include <vka/globals.h>
namespace vka
{

// Render state
RenderState DrawCmd::getRenderState() const
{

	RenderState state;
	state.renderPass        = gState.cache->fetch(pipelineDef.renderPassDefinition);
	state.pipeline          = gState.cache->fetch(pipelineDef);
	state.bindPoint         = VK_PIPELINE_BIND_POINT_GRAPHICS;
	state.pipelineLayoutDef = pipelineDef.pipelineLayoutDefinition;
	state.pipelineLayout    = gState.cache->fetch(pipelineDef.pipelineLayoutDefinition);

	state.framebuffer = gState.framebufferCache->fetch(state.renderPass, attachments);
	state.clearValues = clearValues;
	if (renderArea == VkRect2D_OP{})
	{
		state.renderArea = VkRect2D_OP({0, 0, gState.io.extent.width, gState.io.extent.height});
	}
	else
	{
		state.renderArea = renderArea;
	}
	if (surf.vertexBuffer)
	{
		state.vertexBuffers = {surf.vertexBuffer};
	}
	state.vertexBuffers.insert(state.vertexBuffers.end(), instanceBuffers.begin(), instanceBuffers.end());
	state.indexBuffer = surf.indexBuffer;
	return state;
}

RenderState ComputeCmd::getRenderState() const
{
	RenderState state{};
	state.pipeline          = gState.cache->fetch(pipelineDef);
	state.bindPoint         = VK_PIPELINE_BIND_POINT_COMPUTE;
	state.pipelineLayoutDef = pipelineDef.pipelineLayoutDefinition;
	return state;
}



// Constructors
ComputeCmd::ComputeCmd(uint32_t taskSize, const std::string path, std::vector<ShaderArgs> args)
{
	glm::uvec3 workGroupSize                        = {128, 1, 1};
	glm::uvec3 resolution                           = {taskSize, 1, 1};
	workGroupCount                       = getWorkGroupCount(workGroupSize, resolution);
	pipelineDef.specialisationEntrySizes = glm3VectorSizes();
	pipelineDef.specializationData       = getByteVector(workGroupSize);
	pipelineDef.shaderDef                = ShaderDefinition(path, args);
}

ComputeCmd::ComputeCmd(glm::uvec2 taskSize, std::string path, std::vector<ShaderArgs> args)
{
	glm::uvec3 workGroupSize                        = {32, 32, 1};
	glm::uvec3 resolution                           = {taskSize.x, taskSize.y, 1};
	workGroupCount                       = getWorkGroupCount(workGroupSize, resolution);
	pipelineDef.specialisationEntrySizes = glm3VectorSizes();
	pipelineDef.specializationData       = getByteVector(workGroupSize);
	pipelineDef.shaderDef                = ShaderDefinition(path, args);
}

ComputeCmd::ComputeCmd(VkExtent2D taskSize, std::string path, std::vector<ShaderArgs> args = {})
{
	glm::uvec3 workGroupSize                        = {32, 32, 1};
	glm::uvec3 resolution                           = {taskSize.width, taskSize.height, 1};
	workGroupCount                       = getWorkGroupCount(workGroupSize, resolution);
	pipelineDef.specialisationEntrySizes = glm3VectorSizes();
	pipelineDef.specializationData       = getByteVector(workGroupSize);
	pipelineDef.shaderDef                = ShaderDefinition(path, args);
}

ComputeCmd::ComputeCmd(glm::uvec3 taskSize, std::string path, std::vector<ShaderArgs> args)
{
	glm::uvec3 workGroupSize                        = {8, 8, 8};
	glm::uvec3 resolution                           = taskSize;
	workGroupCount                       = getWorkGroupCount(workGroupSize, resolution);
	pipelineDef.specialisationEntrySizes = glm3VectorSizes();
	pipelineDef.specializationData       = getByteVector(workGroupSize);
	pipelineDef.shaderDef                = ShaderDefinition(path, args);
}



DrawCmd::DrawCmd()
{
	instanceCount = 1;
	pipelineDef   = defaultRasterizationPipeline();
}



void DrawCmd::pushDepthAttachment(Image depthImage, bool clear, VkBool32 enableWrite, VkCompareOp compareOp)
{
	addDepthAttachment(pipelineDef, depthImage, enableWrite, compareOp, clear);
	attachments.push_back(depthImage);
	clearValues.push_back(ClearValue::max_depth());
}

void DrawCmd::pushColorAttachment(Image image, ClearValue clearValue,
                                  VkImageLayout layoutIn, VkImageLayout layoutOut,
                                  BlendOperation colorBlendOp, BlendOperation alphaBlendOp)
{
	addBlendColorAttachment(pipelineDef, image, layoutIn, layoutOut, clearValue.type != CLEAR_VALUE_NONE, colorBlendOp, alphaBlendOp);
	attachments.push_back(image);
	clearValues.push_back(clearValue);
}
void DrawCmd::pushColorAttachment(Image image, VkImageLayout layoutOut, ClearValue clearValue)
{
	addWriteColorAttachment(pipelineDef, image, image->getLayout(), layoutOut, clearValue.type != CLEAR_VALUE_NONE);
	attachments.push_back(image);
	clearValues.push_back(clearValue);
}

void DrawCmd::pushColorAttachment(Image image, ClearValue clearValue)
{
	addWriteColorAttachment(pipelineDef, image, image->getLayout(), image->getLayout(), clearValue.type != CLEAR_VALUE_NONE);
	attachments.push_back(image);
	clearValues.push_back(clearValue);
}

void DrawCmd::pushDescriptor(BufferRef buffer, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, type, shaderStage);
	descriptors.push_back(Descriptor(buffer, type, shaderStage));
}
void DrawCmd::pushDescriptor(Image image, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, type, shaderStage);
	descriptors.push_back(Descriptor(image, type, shaderStage));
}

void DrawCmd::pushDescriptor(const SamplerDefinition sampler, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_SAMPLER, shaderStage);
	descriptors.push_back(Descriptor(sampler, shaderStage));
}

void DrawCmd::pushDescriptor(const SamplerDefinition sampler, Image image, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, shaderStage);
	descriptors.push_back(Descriptor(sampler, image, shaderStage));
}

void DrawCmd::pushDescriptor(std::vector<BufferRef> buffers, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, type, shaderStage);
	descriptors.push_back(Descriptor(buffers, type, shaderStage));
}

void DrawCmd::pushDescriptor(std::vector<Image> images, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, type, shaderStage);
	descriptors.push_back(Descriptor(images, type, shaderStage));
}

void DrawCmd::pushDescriptor(std::vector<SamplerDefinition> samplersDefs, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_SAMPLER, shaderStage);
	descriptors.push_back(Descriptor(samplersDefs, shaderStage));
}

void DrawCmd::pushDescriptor(TLASRef as, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, shaderStage);
	descriptors.push_back(Descriptor(as, shaderStage));
}

void DrawCmd::setGeometry(DrawSurface surface)
{
	surf = surface;
	addInput(pipelineDef, surface.vertexLayout, VK_VERTEX_INPUT_RATE_VERTEX);
}

void DrawCmd::pushInstanceData(BufferRef buffer, VertexDataLayout layout)
{
	instanceBuffers.push_back(buffer);
	addInput(pipelineDef, layout, VK_VERTEX_INPUT_RATE_INSTANCE);
}

void ComputeCmd::pushDescriptor(BufferRef buffer, VkDescriptorType type)
{
	addDescriptor(pipelineDef, type);
	descriptors.push_back(Descriptor(buffer, type, VK_SHADER_STAGE_COMPUTE_BIT));
}
void ComputeCmd::pushDescriptor(Image image, VkDescriptorType type)
{
	addDescriptor(pipelineDef, type);
	descriptors.push_back(Descriptor(image, type, VK_SHADER_STAGE_COMPUTE_BIT));
}

void ComputeCmd::pushDescriptor(const SamplerDefinition sampler)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_SAMPLER);
	descriptors.push_back(Descriptor(sampler, VK_SHADER_STAGE_COMPUTE_BIT));
}

void ComputeCmd::pushDescriptor(const SamplerDefinition sampler, Image image)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	descriptors.push_back(Descriptor(sampler, image, VK_SHADER_STAGE_COMPUTE_BIT));
}

void ComputeCmd::pushDescriptor(std::vector<BufferRef> buffers, VkDescriptorType type)
{
	addDescriptor(pipelineDef, type);
	descriptors.push_back(Descriptor(buffers, type, VK_SHADER_STAGE_COMPUTE_BIT));
}

void ComputeCmd::pushDescriptor(std::vector<Image> images, VkDescriptorType type)
{
	addDescriptor(pipelineDef, type);
	descriptors.push_back(Descriptor(images, type, VK_SHADER_STAGE_COMPUTE_BIT));
}

void ComputeCmd::pushDescriptor(std::vector<SamplerDefinition> samplersDefs)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_SAMPLER);
	descriptors.push_back(Descriptor(samplersDefs, VK_SHADER_STAGE_COMPUTE_BIT));
}


void ComputeCmd::pushDescriptor(TLASRef as, VkShaderStageFlags shaderStage)
{
	addDescriptor(pipelineDef, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
	descriptors.push_back(Descriptor(as, shaderStage));
}

PipelineCmd::PipelineCmd()
{
	pushConstantsData = nullptr;
}

void PipelineCmd::pushConstant(void *data, VkDeviceSize size)
{
	pushConstantsSizes.push_back(size);
	uint32_t offset;
	if (pushConstantsData == nullptr)
	{
		offset            = 0;
		pushConstantsData = createBuffer(gState.frame->stack, 0, VMA_MEMORY_USAGE_CPU_ONLY, size);
	}
	else
	{
		offset = pushConstantsData->getSize();
		pushConstantsData->changeSize(offset + size);
		pushConstantsData->update();
	}
	void *ptr = pushConstantsData->map(offset);
	memcpy(ptr, data, size);
}


}        // namespace vka
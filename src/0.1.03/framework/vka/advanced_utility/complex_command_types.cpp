#include "complex_commands.h"
#include "complex_command_construction.h"
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





//// ComputeCmd
//void addDescriptor(ComputeCmd &drawCmd, Descriptor *desc, VkDescriptorType type)
//{
//	addDescriptor(drawCmd.pipelineDef, type);
//	drawCmd.descriptors.push_back(desc);
//}
//
//// Draw Cmd
//void addDepthAttachment(DrawCmd &drawCmd, Image depthImage, bool clear, VkBool32 enableWrite, VkCompareOp compareOp)
//{
//	addDepthAttachment(drawCmd.pipelineDef, depthImage, enableWrite, compareOp, clear);
//	drawCmd.attachments.push_back(depthImage);
//	drawCmd.clearValues.push_back(VK_CLEAR_COLOR_MAX_DEPTH);
//}
//
//void addColorAttachment(DrawCmd &drawCmd, Image image, ClearValue clearValue,
//                        VkImageLayout layoutIn, VkImageLayout layoutOut,
//                        BlendOperation colorBlendOp, BlendOperation alphaBlendOp)
//{
//	addBlendColorAttachment(drawCmd.pipelineDef, image, layoutIn, layoutOut, clearValue.type != CLEAR_VALUE_NONE, colorBlendOp, alphaBlendOp);
//	drawCmd.attachments.push_back(image);
//	drawCmd.clearValues.push_back(clearValue);
//}
//void addColorAttachment(DrawCmd &drawCmd, Image image, VkImageLayout layoutOut, ClearValue clearValue = {})
//{
//	addWriteColorAttachment(drawCmd.pipelineDef, image, image->getLayout(), layoutOut, clearValue.type != CLEAR_VALUE_NONE);
//	drawCmd.attachments.push_back(image);
//	drawCmd.clearValues.push_back(clearValue);
//}
//
//void addColorAttachment(DrawCmd &drawCmd, Image image, ClearValue clearValue = {})
//{
//	addWriteColorAttachment(drawCmd.pipelineDef, image, image->getLayout(), image->getLayout(), clearValue.type != CLEAR_VALUE_NONE);
//	drawCmd.attachments.push_back(image);
//	drawCmd.clearValues.push_back(clearValue);
//}
//
//void addDescriptor(DrawCmd &drawCmd, IDescriptor *desc, VkDescriptorType type, VkShaderStageFlags shaderStage)
//{
//	addDescriptor(drawCmd.pipelineDef, shaderStage, type);
//	drawCmd.descriptors.push_back(desc);
//}

}
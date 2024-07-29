#include "complex_commands.h"
#include <vka/globals.h>
namespace vka
{
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
}
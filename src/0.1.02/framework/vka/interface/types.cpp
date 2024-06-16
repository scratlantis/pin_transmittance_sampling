#include "types.h"
#include <vka/state_objects/global_state.h>
using namespace vka;

RenderState DrawCmd::getRenderState() const
{
	{
		RenderState state;
		state.framebuffer       = framebuffer;
		state.clearValues       = clearValues;
		if (renderArea == VkRect2D_OP{})
		{
			state.renderArea = VkRect2D_OP({0, 0, gState.io.extent.width, gState.io.extent.height});
		}
		else
		{
			state.renderArea        = renderArea;
		}
		state.renderPass        = gState.cache->fetch(pipelineDef.renderPassDefinition);
		state.pipeline          = gState.cache->fetch(pipelineDef);
		state.bindPoint         = VK_PIPELINE_BIND_POINT_GRAPHICS;
		state.pipelineLayoutDef = pipelineDef.pipelineLayoutDefinition;
		state.vertexBuffers     = {model.vertexBuffer};
		state.vertexBuffers.insert(state.vertexBuffers.end(), instanceBuffers.begin(), instanceBuffers.end());
		state.indexBuffer = model.indexBuffer;
		return state;
	};
}

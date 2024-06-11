#include "DrawCall.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
RenderState DrawCall::getRenderState() const
{
	{
		RenderState state;
		state.framebuffer = framebuffer;
		state.clearValues = clearValues;
		state.renderArea  = renderArea;
		state.renderPass  = gState.cache->fetch(pipelineDef.renderPassDefinition);
		state.pipeline    = gState.cache->fetch(pipelineDef);
		state.bindPoint   = VK_PIPELINE_BIND_POINT_GRAPHICS;
		state.pipelineLayoutDef = pipelineDef.pipelineLayoutDefinition;
		state.vertexBuffers     = {model.vertexBuffer};
		state.vertexBuffers.insert(state.vertexBuffers.end(), instanceBuffers.begin(), instanceBuffers.end());
		state.indexBuffer       = model.indexBuffer;
		return state;
	};
}

std::vector<VkClearValue> RenderState::getClearValues() const
{
	std::vector<VkClearValue> clearValues;
	for (size_t i = 0; i < this->clearValues.size(); i++)
	{
		clearValues.push_back(this->clearValues[i].value);
	}
	return clearValues;
}
}        // namespace vka

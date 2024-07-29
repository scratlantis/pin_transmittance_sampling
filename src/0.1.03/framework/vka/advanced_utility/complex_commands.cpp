#include "complex_commands.h"
#include <vka/core/core_utility/general_commands.h>
#include <vka/globals.h>


namespace vka
{

void cmdCompute(CmdBuffer cmdBuf, const ComputeCmd &computeCmd)
{
	cmdClearState(cmdBuf);
	cmdBuf->renderState = computeCmd.getRenderState();
	cmdBindPipeline(cmdBuf);
	cmdPushDescriptors(cmdBuf, 0, computeCmd.descriptors);        // only one descriptor set for now
	cmdPushConstants(cmdBuf, computeCmd.pushConstantsSizes, computeCmd.pushConstantsData);
	cmdDispatch(cmdBuf, computeCmd.workGroupCount);
}


void cmdDraw(CmdBuffer cmdBuf, const DrawCmd &drawCmd)
{
	VKA_ASSERT(drawCmd.pipelineDef.subpass == 0);

	RenderState newRenderState = drawCmd.getRenderState();
	uint32_t    diffBits       = cmdBuf->renderState.calculateDifferenceBits(newRenderState);
	// End render pass if needed
	if (diffBits & RENDER_STATE_ACTION_BIT_END_RENDER_PASS)
	{
		cmdEndRenderPass(cmdBuf);
	}
	// Assure images are in correct layout
	if (diffBits & RENDER_STATE_ACTION_BIT_START_RENDER_PASS)
	{
		VKA_ASSERT(drawCmd.attachments.size() == drawCmd.pipelineDef.renderPassDefinition.attachmentDescriptions.size())
		for (size_t i = 0; i < drawCmd.attachments.size(); i++)
		{
			cmdTransitionLayout(cmdBuf, drawCmd.attachments[i], drawCmd.pipelineDef.renderPassDefinition.attachmentDescriptions[i].initialLayout);
			drawCmd.attachments[i]->setLayout(drawCmd.pipelineDef.renderPassDefinition.attachmentDescriptions[i].finalLayout);
		}
	}
	// Aquire new render state
	cmdBuf->renderState = newRenderState;
	// Start render pass if needed
	if (diffBits & RENDER_STATE_ACTION_BIT_START_RENDER_PASS)
	{
		cmdStartRenderPass(cmdBuf, cmdBuf->renderState.renderPass, cmdBuf->renderState.framebuffer, cmdBuf->renderState.renderArea, cmdBuf->renderState.getClearValues());
	}
	// Bind pipeline if needed
	if (diffBits & RENDER_STATE_ACTION_BIT_BIND_PIPELINE)
	{
		cmdBindPipeline(cmdBuf);
	}
	// for now, only one descriptor set, always rebind
	cmdPushDescriptors(cmdBuf, 0, drawCmd.descriptors);

	if (diffBits & RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER && !cmdBuf->renderState.vertexBuffers.empty())
	{
		cmdBindVertexBuffers(cmdBuf);
	}

	if (cmdBuf->renderState.indexBuffer)
	{
		cmdBindIndexBuffer(cmdBuf, drawCmd.surf.offset);
		cmdDrawIndexed(cmdBuf, drawCmd.surf.count, drawCmd.instanceCount, 0, 0, 0);
	}
	else
	{
		cmdDraw(cmdBuf, drawCmd.surf.count, drawCmd.instanceCount, 0, 0);
	}
}

}
#pragma once
#include <vka/resource_objects/resource_common.h>
#include <vka/state_objects/global_state.h>
#include "types.h"

namespace vka
{
VkaCommandBuffer vkaCreateCommandBuffer(IResourcePool *pPool)
{
	VkaCommandBuffer cmdBuffer = new CmdBuffer_I(pPool, CMD_BUF_CAPABILITY_MASK_UNIVERSAL, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);
	return cmdBuffer;
}

VkaCommandBuffer vkaExecuteImmediat(VkaCommandBuffer cmdBuffer, VkQueue queue = gState.device.universalQueues[0])
{
	   cmdBuffer->end();
	   submit(cmdBuffer->getHandle(), queue, {});
	   vkDeviceWaitIdle(gState.device.logical);
	   return cmdBuffer;
}

void vkaSubmit(std::vector<VkaCommandBuffer> cmdBufs, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
{
	std::vector<VkCommandBuffer> vkCmdBufs;
	for (size_t i = 0; i < cmdBufs.size(); i++)
	{
		cmdBufs[i]->end();
		cmdBufs[i]->garbageCollect();
		vkCmdBufs.push_back(cmdBufs[i]->getHandle());
	}
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.waitSemaphoreCount   = syncInfo.waitSemaphores.size();
	submit.pWaitSemaphores      = syncInfo.waitSemaphores.data();
	submit.pWaitDstStageMask    = syncInfo.waitDstStageMask.data();
	submit.signalSemaphoreCount = syncInfo.signalSemaphores.size();
	submit.pSignalSemaphores    = syncInfo.signalSemaphores.data();
	submit.pCommandBuffers      = vkCmdBufs.data();
	submit.commandBufferCount   = vkCmdBufs.size();
	VK_CHECK(vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence));
}


}        // namespace vka

#define VKA_IMMEDIATE(CODE)                                                \
	VkaCommandBuffer cmdBuf = vkaCreateCommandBuffer(gState.frame->stack); \
	CODE;                                                                  \
	vkaExecuteImmediate(cmdBuf);
#include "CmdBuffer.h"

namespace vka
{

void commitCmdBuffers(
    std::vector<CmdBuffer> cmdBufs, ResourceTracker* pTracker, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
{
	std::vector<VkCommandBuffer> vkCmdBufs;
	for (size_t i = 0; i < cmdBufs.size(); i++)
	{
		cmdBufs[i].end();
		cmdBufs[i].move(pTracker);
		vkCmdBufs.push_back(cmdBufs[i].handle);
	}
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.waitSemaphoreCount   = syncInfo.waitSemaphores.size();
	submit.pWaitSemaphores      = syncInfo.waitSemaphores.data();
	submit.pWaitDstStageMask    = syncInfo.waitDstStageMask.data();
	submit.signalSemaphoreCount = syncInfo.signalSemaphores.size();
	submit.pSignalSemaphores    = syncInfo.signalSemaphores.data();
	submit.pCommandBuffers      = vkCmdBufs.data();
	submit.commandBufferCount   = vkCmdBufs.size();
	ASSERT_VULKAN(vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence));
}


CmdBuffer::CmdBuffer()
{
	isRecording = true;
}

CmdBuffer::~CmdBuffer()
{
}
}
#include "state_functionality.h"

using namespace vka;
void vkaSwapBuffers(std::vector<VkaCommandBuffer> cmdBufs)
{
	SubmitSynchronizationInfo syncInfo = gState.acquireNextSwapchainImage();
	vkaSubmit(cmdBufs, gState.device.universalQueues[0], syncInfo);
	vkDeviceWaitIdle(gState.device.logical);
	gState.presentFrame();
	gState.nextFrame();
}
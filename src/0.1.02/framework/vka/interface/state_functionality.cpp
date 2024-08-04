#include "state_functionality.h"

using namespace vka;
void vkaSwapBuffers(std::vector<VkaCommandBuffer> cmdBufs)
{
	SubmitSynchronizationInfo syncInfo = gState.acquireNextSwapchainImage();
	vkaSubmit(cmdBufs, gState.device.universalQueues[0], syncInfo);
	gState.presentFrame();
	gState.nextFrame();
}

vka::VkRect2D_OP vkaGetScissorRect(float x, float y, float width, float height)
{
	return vka::VkRect2D_OP{{{0, 0}, gState.io.extent}} * Rect2D<float>{ x, y, std::min(width,1.0f-x), std::min(height,1.0f-y)};
}

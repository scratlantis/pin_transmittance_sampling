#pragma once
#include "types.h"
#include <vka/resource_objects/resource_common.h>
#include <vka/state_objects/global_state.h>
#include "cmd_buffer_functionality.h"

namespace vka
{
void vkaSwapBuffers(std::vector<VkaCommandBuffer> cmdBufs)
{
	SubmitSynchronizationInfo syncInfo = gState.acquireNextSwapchainImage();
	vkaSubmit(cmdBufs, gState.device.universalQueues[0], syncInfo);
	gState.presentFrame();
	gState.nextFrame();
}
}
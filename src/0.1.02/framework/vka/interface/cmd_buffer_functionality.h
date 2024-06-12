#pragma once
#include <vka/resource_objects/resource_common.h>
#include <vka/state_objects/global_state.h>
#include "types.h"


VkaCommandBuffer vkaCreateCommandBuffer(vka::IResourcePool *pPool);

VkaCommandBuffer vkaExecuteImmediat(VkaCommandBuffer cmdBuffer, VkQueue queue = gState.device.universalQueues[0]);

void vkaSubmit(std::vector<VkaCommandBuffer> cmdBufs, VkQueue queue, const vka::SubmitSynchronizationInfo syncInfo);

#define VKA_IMMEDIATE(CODE)                                                \
	VkaCommandBuffer cmdBuf = vkaCreateCommandBuffer(gState.frame->stack); \
	CODE;                                                                  \
	vkaExecuteImmediat(cmdBuf);
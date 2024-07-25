#pragma once
#include "types.h"
#include <vka/core/core_state/IResourcePool.h>

namespace vka
{
CmdBuffer createCmdBuffer(IResourcePool *pPool);
void      executeImmediat(CmdBuffer cmdBuffer, VkQueue queue);
void      submit(std::vector<CmdBuffer> cmdBufs, VkQueue queue, const vka::SubmitSynchronizationInfo syncInfo);
}		// namespace vka
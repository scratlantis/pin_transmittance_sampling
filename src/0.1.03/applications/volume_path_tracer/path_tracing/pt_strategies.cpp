#include "pt_strategies.h"

void ReferencePathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo)
{
	cmdFill(cmdBuf, localTarget, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(1.0, 0.0, 0.0, 1.0));        // Mock
}

void PinPathTracer::trace(CmdBuffer cmdBuf, Image localTarget, const RenderInfo &renderInfo)
{
	cmdFill(cmdBuf, localTarget, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0, 1.0, 0.0, 1.0));        // Mock
}

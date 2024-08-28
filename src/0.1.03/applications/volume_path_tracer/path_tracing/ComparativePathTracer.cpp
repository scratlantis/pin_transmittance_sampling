#include "ComparativePathTracer.h"

ComparativePathTracer::ComparativePathTracer(float relativeWidth, float relativeHeight)
{
	localTargetA = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localTargetB = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localAccumulationTargetA = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localAccumulationTargetB = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	gState.updateSwapchainAttachments();
}


void ComparativePathTracer::reset(CmdBuffer cmdBuf, PathTraceStrategy *pStrategieA, PathTraceStrategy *pStrategieB)
{
	cmdFill(cmdBuf, localAccumulationTargetA, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0));
	cmdFill(cmdBuf, localAccumulationTargetB, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0));

	this->pStrategieA = pStrategieA;
	this->pStrategieB = pStrategieB;
}

void ComparativePathTracer::render(CmdBuffer cmdBuf, const RenderInfo &renderInfo)
{
	// Render
	pStrategieA->trace(cmdBuf, localTargetA, renderInfo);
	pStrategieB->trace(cmdBuf, localTargetB, renderInfo);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	// Accumulate
	getCmdAccumulate(localTargetA, localAccumulationTargetA, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
	getCmdAccumulate(localTargetB, localAccumulationTargetB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
}

void ComparativePathTracer::showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea)
{
	VkRect2D_OP srcAreaA = VkRect2D_OP(localAccumulationTargetA->getExtent2D());
	srcAreaA.extent.width *= splittCoef;
	VkRect2D_OP srcAreaB = VkRect2D_OP(localAccumulationTargetB->getExtent2D());
	srcAreaB.offset.x += srcAreaA.extent.width;
	srcAreaB.extent.width -= srcAreaA.extent.width;

	VkRect2D_OP dstAreaA = targetArea;
	dstAreaA.extent.width *= splittCoef;
	VkRect2D_OP dstAreaB = targetArea;
	dstAreaB.offset.x += dstAreaA.extent.width;
	dstAreaB.extent.width -= dstAreaA.extent.width;

	// Bar in the middle
	dstAreaA.extent.width -= 1;
	dstAreaB.extent.width -= 1;
	dstAreaB.offset.x += 1;

	if (dstAreaB.extent.width == 0 || srcAreaB.extent.width == 0)
	{
		getCmdNormalize(localAccumulationTargetA, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaA, dstAreaA).exec(cmdBuf);
	}
	else if (dstAreaA.extent.width > 0 && srcAreaA.extent.width > 0)
	{
		getCmdNormalize(localAccumulationTargetA, target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcAreaA, dstAreaA).exec(cmdBuf);
		getCmdNormalize(localAccumulationTargetB, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaB, dstAreaB).exec(cmdBuf);
	}
	else
	{
		getCmdNormalize(localAccumulationTargetB, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaB, dstAreaB).exec(cmdBuf);
	}
}

void ComparativePathTracer::showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea)
{
	getCmdNormalizeDiff(localAccumulationTargetA, localAccumulationTargetB, target,
	                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkRect2D_OP(localAccumulationTargetA->getExtent2D()), targetArea)
	    .exec(cmdBuf);
}


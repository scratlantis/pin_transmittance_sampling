#include "ComparativePathTracer.h"

ComparativePathTracer::ComparativePathTracer(float relativeWidth, float relativeHeight)
{
	localTargetA = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localTargetB = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localAccumulationTargetA = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localAccumulationTargetB = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	gState.updateSwapchainAttachments();
}


void ComparativePathTracer::clearAccumulationTargets(CmdBuffer cmdBuf)
{
	cmdFill(cmdBuf, localAccumulationTargetA, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0));
	cmdFill(cmdBuf, localAccumulationTargetB, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0));
}

void ComparativePathTracer::renderSplitView(CmdBuffer cmdBuf, PathTraceStrategy *pStrategieA, PathTraceStrategy *pStrategieB, Image target, float splittCoef, VkRect2D_OP targetArea, const RenderInfo &renderInfo)
{
	clearAccumulationTargets(cmdBuf);

	// Render
	pStrategieA->trace(cmdBuf, localTargetA, renderInfo);
	pStrategieB->trace(cmdBuf, localTargetB, renderInfo);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	// Accumulate
	getCmdAccumulate(localTargetA, localAccumulationTargetA, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
	getCmdAccumulate(localTargetB, localAccumulationTargetB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);


	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

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

	getCmdNormalize(localAccumulationTargetA, target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcAreaA, dstAreaA).exec(cmdBuf);
	getCmdNormalize(localAccumulationTargetB, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaB, dstAreaB).exec(cmdBuf);
}


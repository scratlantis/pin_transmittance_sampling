#include "ImageEstimatorComparator.h"
#include <vka/advanced_utility/misc_utility.h>
#include <vka/globals.h>
#include <vka/specialized_utility/draw_2D.h>
namespace vka
{
ImageEstimatorComparator::ImageEstimatorComparator(VkFormat format, float relWidth, float relHeight)
{
	Image* imgs[4] = {&localTargetLeft, &localTargetRight, &localAccumulationTargetLeft, &localAccumulationTargetRight};
	for (auto img : imgs)
	{
		*img = createSwapchainAttachment(format,
		                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		                                 VK_IMAGE_LAYOUT_GENERAL, relWidth, relHeight);
	}
	tqManager = TimeQueryManager(gState.heap, 2);
	mseResources = MSEComputeResources(format, gState.heap);
	mseBuffer    = createBuffer(gState.hostCachedHeap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, sizeof(float));
}
void ImageEstimatorComparator::cmdReset(CmdBuffer cmdBuf)
{
	cmdFill(cmdBuf, localAccumulationTargetLeft, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, glm::vec4(0.0));
	cmdFill(cmdBuf, localAccumulationTargetRight, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, glm::vec4(0.0));
	tqManager.cmdResetQueryPool(cmdBuf);
}
void ImageEstimatorComparator::showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea)
{
	VkRect2D_OP srcAreaLeft = VkRect2D_OP(localAccumulationTargetLeft->getExtent2D());
	srcAreaLeft.extent.width *= splittCoef;
	VkRect2D_OP srcAreaRight = VkRect2D_OP(localAccumulationTargetRight->getExtent2D());
	srcAreaRight.offset.x += srcAreaLeft.extent.width;
	srcAreaRight.extent.width -= srcAreaLeft.extent.width;

	VkRect2D_OP dstAreaLeft = targetArea;
	dstAreaLeft.extent.width *= splittCoef;
	VkRect2D_OP dstAreaRight = targetArea;
	dstAreaRight.offset.x += dstAreaLeft.extent.width;
	dstAreaRight.extent.width -= dstAreaLeft.extent.width;

	// Bar in the middle
	if (dstAreaLeft.extent.width != 0) dstAreaLeft.extent.width -= 1;
	if (dstAreaRight.extent.width != 0) dstAreaRight.extent.width -= 1;
	dstAreaRight.offset.x += 1;

	if (dstAreaRight.extent.width == 0 || dstAreaRight.extent.width == 0)
	{
		getCmdNormalize(localAccumulationTargetLeft, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaLeft, dstAreaLeft).exec(cmdBuf);
	}
	else if (dstAreaLeft.extent.width > 0 && srcAreaLeft.extent.width > 0)
	{
		srcAreaLeft.extent.width -= 1;
		srcAreaRight.extent.width -= 1;
		srcAreaRight.offset.x += 1;

		getCmdNormalize(localAccumulationTargetLeft, target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcAreaLeft, dstAreaLeft).exec(cmdBuf);
		getCmdNormalize(localAccumulationTargetRight, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaRight, dstAreaRight).exec(cmdBuf);
	}
	else
	{
		srcAreaRight.extent.width -= 1;
		srcAreaRight.offset.x += 1;
		getCmdNormalize(localAccumulationTargetRight, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaRight, dstAreaRight).exec(cmdBuf);
	}
}
void ImageEstimatorComparator::showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea)
{
	getCmdNormalizeDiff(localAccumulationTargetLeft, localAccumulationTargetRight, target,
	                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkRect2D_OP(localAccumulationTargetLeft->getExtent2D()), targetArea)
	    .exec(cmdBuf);
}
float ImageEstimatorComparator::getMSE()
{
	Buffer hostMseBuf;
	float  mse = 0;
	if (gState.hostCache->fetch(mseBuffer, hostMseBuf))
	{
		mse = *static_cast<float *>(hostMseBuf->map());
	}
	return mse;
}
}        // namespace vka
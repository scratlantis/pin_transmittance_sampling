#include "ComparativePathTracer.h"

ComparativePathTracer::ComparativePathTracer(float relativeWidth, float relativeHeight,
                                             Buffer lineSegmentInstanceBuffer, uint32_t lineSegmentCount)
{
	localTargetA = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localTargetB = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localAccumulationTargetA = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	localAccumulationTargetB = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, relativeWidth, relativeHeight);
	gState.updateSwapchainAttachments();

	mseRes    = MSEComputeResources(VK_FORMAT_R32G32B32A32_SFLOAT, gState.heap);
	mseBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, sizeof(float));

	tqManager = TimeQueryManager({TQ_PATH_TRACER_A, TQ_PATH_TRACER_B});

	this->lineSegmentInstanceBuffer = lineSegmentInstanceBuffer;
	this->lineSegmentCount          = lineSegmentCount;
	lineSegmentBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLLineSegment) * lineSegmentCount);
}

void ComparativePathTracer::destroy()
{
	tqManager.destroy();
	lineSegmentBuffer->garbageCollect();
}

ComputeCmd ComparativePathTracer::getCmdWriteLineSegments()
{
	ComputeCmd cmd(lineSegmentCount, shaderPath + "misc/line_segment_to_instance.comp", {{"INPUT_SIZE", lineSegmentCount}});
	cmd.pushDescriptor(lineSegmentBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(lineSegmentInstanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	return cmd;
}


void ComparativePathTracer::reset(CmdBuffer cmdBuf, PathTraceStrategy *pStrategieA, PathTraceStrategy *pStrategieB)
{
	cmdFill(cmdBuf, localAccumulationTargetA, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0));
	cmdFill(cmdBuf, localAccumulationTargetB, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0));

	this->pStrategieA = pStrategieA;
	this->pStrategieB = pStrategieB;
}

GVar gvar_timing_left{"Timing left : %.8f ms", 0.0f, GVAR_DISPLAY_VALUE, METRICS};
GVar gvar_timing_right{"Timing right : %.8f ms", 0.0f, GVAR_DISPLAY_VALUE, METRICS};

void ComparativePathTracer::render(CmdBuffer cmdBuf, const RenderInfo &renderInfo)
{
	
	// Render

	cmdFillBuffer(cmdBuf, lineSegmentBuffer, 0, sizeof(GLSLLineSegment) * lineSegmentCount, 0);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);
	if (timeQueryFinished)
	{
		tqManager.cmdResetQueryPool(cmdBuf);

		tqManager.startTiming(cmdBuf, TQ_PATH_TRACER_A, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		pStrategieA->trace(cmdBuf, localTargetA, renderInfo, lineSegmentBuffer);
		tqManager.endTiming(cmdBuf, TQ_PATH_TRACER_A, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

		tqManager.startTiming(cmdBuf, TQ_PATH_TRACER_B, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		pStrategieB->trace(cmdBuf, localTargetB, renderInfo, lineSegmentBuffer);
		tqManager.endTiming(cmdBuf, TQ_PATH_TRACER_B, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
	}
	else
	{
		pStrategieA->trace(cmdBuf, localTargetA, renderInfo, lineSegmentBuffer);
		pStrategieB->trace(cmdBuf, localTargetB, renderInfo, lineSegmentBuffer);
	}
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	getCmdWriteLineSegments().exec(cmdBuf);

	timeQueryFinished = tqManager.updateTimings();

	gvar_timing_left.val.v_float = tqManager.timings[TQ_PATH_TRACER_A];
	gvar_timing_right.val.v_float = tqManager.timings[TQ_PATH_TRACER_B];

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
		srcAreaA.extent.width -= 1;
		srcAreaB.extent.width -= 1;
		srcAreaB.offset.x += 1;

		getCmdNormalize(localAccumulationTargetA, target, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcAreaA, dstAreaA).exec(cmdBuf);
		getCmdNormalize(localAccumulationTargetB, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaB, dstAreaB).exec(cmdBuf);
	}
	else
	{
		srcAreaB.extent.width -= 1;
		srcAreaB.offset.x += 1;
		getCmdNormalize(localAccumulationTargetB, target, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, srcAreaB, dstAreaB).exec(cmdBuf);
	}
}

void ComparativePathTracer::showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea)
{
	getCmdNormalizeDiff(localAccumulationTargetA, localAccumulationTargetB, target,
	                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkRect2D_OP(localAccumulationTargetA->getExtent2D()), targetArea)
	    .exec(cmdBuf);
}

float ComparativePathTracer::computeMSE(CmdBuffer cmdBuf)
{
	cmdComputeMSE(cmdBuf, localAccumulationTargetA, localAccumulationTargetB, mseBuffer, &mseRes);
	float *data = static_cast<float *>(mseBuffer->map());
	return *data;
}


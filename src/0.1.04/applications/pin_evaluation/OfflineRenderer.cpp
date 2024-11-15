#include "OfflineRenderer.h"

const float referenceRMStepSize = 0.01;

OfflineRenderer::OfflineRenderer()
{
	this->pTraceResourceCache = new ResourceCache();
	this->pPool = new ResourcePool();
}

bool OfflineRenderer::cmdRunTick(CmdBuffer cmdBuf)
{
	if (taskQueue.empty())
	{
		return false;
	}
	OfflineRenderTask &task = taskQueue.front();
	if (newTask)
	{
		iec            = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, task.resolution);
		newTask = false;
		execCntRef = 0;
		execCntLeft = 0;
		execCntRight = 0;
		traceResources = cmdLoadResources(cmdBuf, pPool);
		TraceArgs commonArgs{};
		commonArgs.resources = traceResources;
		commonArgs.sceneParams = task.sceneParams;
		commonArgs.config = task.config;

		argsRef   = TraceArgs(pTraceResourceCache, pPool, &execCntRef);
		argsRef.update(commonArgs);
		argsRef.config.rayMarchStepSize = referenceRMStepSize;

		argsLeft  = TraceArgs(pTraceResourceCache, pPool, &execCntLeft);
		argsLeft.update(commonArgs);
		argsLeft.cvsArgs = task.cvsArgsLeft;

		argsRight = TraceArgs(pTraceResourceCache, pPool, &execCntRight);
		argsRight.update(commonArgs);
		argsRight.cvsArgs = task.cvsArgsRight;
	}


}

void OfflineRenderer::addTask(OfflineRenderTask task)
{
	taskQueue.push_back(task);
}



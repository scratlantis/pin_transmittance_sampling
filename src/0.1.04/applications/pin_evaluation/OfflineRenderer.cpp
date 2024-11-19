#include "OfflineRenderer.h"
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

const float referenceRMStepSize = 0.01;

OfflineRenderer::OfflineRenderer()
{
	this->pTraceResourceCache = new ResourceCache();
	this->pPool = new ResourcePool();
	this->state = OFFLINE_RENDER_STATE_IDLE;
	refTask                   = {};
	leftTask                  = {};
	rightTask                 = {};
}

uint32_t OfflineRenderer::getTaskQueueSize()
{
	return taskQueue.size();
}


float OfflineRenderer::getTaskProgress()
{
	if (taskQueue.empty())
	{
		return 1.0;
	}
	OfflineRenderTask &task      = taskQueue.front();
	float              totalTime = task.renderTimeRef + task.renderTimeCompare * 2;
	// clang-format off
	float              progress  = 
		refTask.avgSampleTime * refTask.execCnt
		+ leftTask.avgSampleTime * leftTask.execCnt
		+ rightTask.avgSampleTime * rightTask.execCnt;
	// clang-format on
	return glm::clamp(progress / totalTime, 0.f, 1.f);
}




bool OfflineRenderer::cmdRunTick(CmdBuffer cmdBuf)
{
	if (taskQueue.empty())
	{
		return false;
	}
	OfflineRenderTask &task = taskQueue.front();
	if (state == OFFLINE_RENDER_STATE_IDLE)
	{
		iec            = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, task.resolution);
		iec.cmdReset(cmdBuf);
		refTask.execCnt        = 0;
		leftTask.execCnt       = 0;
		rightTask.execCnt      = 0;
		TraceArgs commonArgs{};
		commonArgs.resources.cmdLoadAll(cmdBuf, pPool);
		commonArgs.sceneParams = task.sceneParams;
		commonArgs.config = task.config;
		commonArgs.config.subSampleMode = 1;

		refTask.reset();
		refTask.args = TraceArgs(pTraceResourceCache, pPool, &refTask.execCnt);
		refTask.args.update(commonArgs);
		refTask.args.config.rayMarchStepSize = referenceRMStepSize;
		refTask.result                       = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		refTask.targetTotalRenderTime		= task.renderTimeRef;

		leftTask.reset();
		leftTask.args  = TraceArgs(pTraceResourceCache, pPool, &leftTask.execCnt);
		leftTask.args.update(commonArgs);
		leftTask.args.cvsArgs = task.cvsArgsLeft;
		leftTask.result       = createImage(pPool, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		leftTask.targetTotalRenderTime = task.renderTimeCompare;

		rightTask.reset();
		rightTask.args = TraceArgs(pTraceResourceCache, pPool, &rightTask.execCnt);
		rightTask.args.update(commonArgs);
		rightTask.args.cvsArgs = task.cvsArgsRight;
		rightTask.result = createImage(pPool, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		rightTask.targetTotalRenderTime = task.renderTimeCompare;

		state = OFFLINE_RENDER_STATE_REF;
		iec.cmdReset(cmdBuf);
	}
	RenderTaskInternal* tasks[3] = {&refTask, &leftTask, &rightTask};
	RenderTaskInternal &internalTask = *tasks[state];
	std::string internalTaskName = task.name + (state == OFFLINE_RENDER_STATE_REF ? "_ref" : state == OFFLINE_RENDER_STATE_LEFT ? "_left" : "_right");
	bool computeMSE = state != OFFLINE_RENDER_STATE_REF;
	iec.cmdRun<TraceArgs>(cmdBuf, cmdTrace, internalTask.args, IEC_TARGET_LEFT, &internalTask.avgSampleTime,
	                      computeMSE ? IEC_FLAGS_NONE: IEC_RUN_NO_MSE);
	internalTask.execCnt++;
	uint32_t frameCycle = internalTask.args.config.subSampleMode * internalTask.args.config.subSampleMode;
	if (computeMSE && internalTask.execCnt % frameCycle == 0)
	{
		internalTask.mse.push_back(iec.getMSE());
	}
	if (internalTask.isComplete())
	{
		printVka("Render %s complete", internalTaskName.c_str());
		iec.cmdShow(cmdBuf, internalTask.result, VkRect2D_OP(task.resolution), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, IEC_TARGET_LEFT);
		
		// Store render result
		Image outImg = internalTask.result;
		if (state == OFFLINE_RENDER_STATE_REF)
		{
			outImg = createImage(pPool, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
			iec.cmdShow(cmdBuf, outImg, VkRect2D_OP(task.resolution), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, IEC_TARGET_LEFT);
		}
		iec.cmdReset(cmdBuf, nullptr, refTask.result);

		std::filesystem::create_directories(resultsPath + task.name);
		std::string internalTaskPath = resultsPath + task.name + "/" + internalTaskName;
		ExportTask  exportTask{};
		exportTask.path         = internalTaskPath + ".png";
		exportTask.pResource    = outImg;
		exportTask.targetFormat = EXPORT_FORMAT_PNG;
		gState.exporter->cmdExport(cmdBuf, exportTask);

		if (state != OFFLINE_RENDER_STATE_REF)
		{
			// Store mse over time data as csv
			Buffer     mseBuffer = createBuffer(pPool, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			cmdWriteCopy(cmdBuf, mseBuffer, internalTask.mse.data(), internalTask.mse.size() * sizeof(float));
			ExportTask exportTask{};
			exportTask.path                 = internalTaskPath + ".csv";
			exportTask.pResource            = mseBuffer;
			exportTask.targetFormat         = EXPORT_FORMAT_CSV;
			exportTask.bufferInfo.format    = EXPORT_BUFFER_FORMAT_FLOAT;
			exportTask.bufferInfo.rowLength = internalTask.mse.size();
			gState.exporter->cmdExport(cmdBuf, exportTask);
		}

		state = static_cast<OfflineRenderState>(state + 1);
		if (state == OFFLINE_RENDER_STATE_IDLE)
		{
			// Write json
			json          j;
			std::ofstream o(resultsPath + task.name + "/" + task.name + "_info.json");
			j["subsample_mode"] = task.config.subSampleMode;
			j["resolution"]     = {task.resolution.width, task.resolution.height};

			j["ref_avg_sample_time"] = refTask.avgSampleTime;
			j["ref_sample_count"]    = refTask.execCnt;

			j["left_avg_sample_time"] = leftTask.avgSampleTime;
			j["left_sample_count"]    = leftTask.execCnt;

			j["right_avg_sample_time"] = rightTask.avgSampleTime;
			j["right_sample_count"]    = rightTask.execCnt;

			o << std::setw(4) << j << std::endl;
			o.close();
			// Clean up
			taskQueue.erase(taskQueue.begin());
			pPool->dump(gState.frame->stack);
			iec.garbageCollect();
		}
	}


}

void OfflineRenderer::addTask(OfflineRenderTask task)
{
	taskQueue.push_back(task);
}

void OfflineRenderer::destroy()
{
	pTraceResourceCache->clearAll();
	delete pTraceResourceCache;
	pPool->clear();
	delete pPool;
}



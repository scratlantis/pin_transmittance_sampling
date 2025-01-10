#include "SinglePassOfflineRenderer.h"
#include "ui.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const float referenceRMStepSize = 0.01;

extern GVar gvar_eval_custom_export_path;
extern GVar gvar_eval_use_custom_export_path;

GVar gvar_reset_offline_renderer{"Reset offline renderer", false, GVAR_EVENT, GUI_CAT_EVALUATION};

bool SIOfflineRenderer::cmdRunTick(CmdBuffer cmdBuf)
{
	if (state == OFFLINE_RENDER_STATE_NEXT && gvar_reset_offline_renderer.val.v_bool)
	{
		state = OFFLINE_RENDER_STATE_IDLE;
		iec.garbageCollect();
	}
	if (taskQueue.empty())
	{
		return false;
	}
	OfflineRenderTask &task = taskQueue.front();
	if (state == OFFLINE_RENDER_STATE_IDLE)
	{
		waitIdle();
		this->pTraceResourceCache->clearAll();
		this->pPool->clear();

		iec = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, task.resolution);
		iec.cmdReset(cmdBuf);
		refTask.execCnt   = 0;
		leftTask.execCnt  = 0;
		rightTask.execCnt = 0;
		TraceArgs commonArgs{};
		commonArgs.resources.cmdLoadAll(cmdBuf, pPool, this->pTraceResourceCache);
		commonArgs.sceneParams          = task.sceneParams;
		commonArgs.config               = task.config;
		commonArgs.config.subSampleMode = 1;

		refTask.reset();
		refTask.args = TraceArgs(pTraceResourceCache, pPool, &refTask.execCnt);
		refTask.args.update(commonArgs);
		refTask.args.config.rayMarchStepSize = referenceRMStepSize;
		refTask.result                       = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		refTask.targetTotalRenderTime        = task.renderTimeRef;
		refTask.targetTotalSamples           = std::numeric_limits<uint32_t>::max();
		refTask.args.config.seed += 0x543419;

		leftTask.reset();
		leftTask.args = TraceArgs(pTraceResourceCache, pPool, &leftTask.execCnt);
		leftTask.args.update(commonArgs);
		leftTask.args.cvsArgs          = task.cvsArgsLeft;
		leftTask.result                = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		leftTask.targetTotalRenderTime = (task.mode == OFFLINE_RENDER_MODE_EQUAL_TIME) ? task.renderTimeCompare : std::numeric_limits<float>::max();
		leftTask.targetTotalSamples    = (task.mode == OFFLINE_RENDER_MODE_EQUAL_SAMPLES) ? task.invocationsCompare : std::numeric_limits<uint32_t>::max();
		leftTask.args.config.seed += 0x47283;

		rightTask.reset();
		rightTask.args = TraceArgs(pTraceResourceCache, pPool, &rightTask.execCnt);
		rightTask.args.update(commonArgs);
		rightTask.args.cvsArgs          = task.cvsArgsRight;
		rightTask.result                = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		rightTask.targetTotalRenderTime = (task.mode == OFFLINE_RENDER_MODE_EQUAL_TIME) ? task.renderTimeCompare : std::numeric_limits<float>::max();
		rightTask.targetTotalSamples    = (task.mode == OFFLINE_RENDER_MODE_EQUAL_SAMPLES) ? task.invocationsCompare : std::numeric_limits<uint32_t>::max();
		rightTask.args.config.seed += 0x146123;

		state = OFFLINE_RENDER_STATE_REF;
		iec.cmdReset(cmdBuf);
	}
	if (state == OFFLINE_RENDER_STATE_NEXT)
	{
		waitIdle();
		this->pTraceResourceCache->clearAll();
		this->pPool->clear();

		//iec = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, task.resolution);
		//iec.cmdReset(cmdBuf);
		refTask.execCnt   = 0;
		leftTask.execCnt  = 0;
		rightTask.execCnt = 0;
		TraceArgs commonArgs{};
		commonArgs.resources.cmdLoadAll(cmdBuf, pPool, this->pTraceResourceCache);
		commonArgs.sceneParams          = task.sceneParams;
		commonArgs.config               = task.config;
		commonArgs.config.subSampleMode = 1;

		refTask.reset();
		refTask.args = TraceArgs(pTraceResourceCache, pPool, &refTask.execCnt);
		refTask.args.update(commonArgs);
		refTask.args.config.rayMarchStepSize = referenceRMStepSize;
		refTask.result                       = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		refTask.targetTotalRenderTime        = task.renderTimeRef;
		refTask.targetTotalSamples           = std::numeric_limits<uint32_t>::max();
		refTask.args.config.seed += 0x543419;

		leftTask.reset();
		leftTask.args = TraceArgs(pTraceResourceCache, pPool, &leftTask.execCnt);
		leftTask.args.update(commonArgs);
		leftTask.args.cvsArgs          = task.cvsArgsLeft;
		leftTask.result                = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		leftTask.targetTotalRenderTime = (task.mode == OFFLINE_RENDER_MODE_EQUAL_TIME) ? task.renderTimeCompare : std::numeric_limits<float>::max();
		leftTask.targetTotalSamples    = (task.mode == OFFLINE_RENDER_MODE_EQUAL_SAMPLES) ? task.invocationsCompare : std::numeric_limits<uint32_t>::max();
		leftTask.args.config.seed += 0x47283;

		rightTask.reset();
		rightTask.args = TraceArgs(pTraceResourceCache, pPool, &rightTask.execCnt);
		rightTask.args.update(commonArgs);
		rightTask.args.cvsArgs          = task.cvsArgsRight;
		rightTask.result                = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		rightTask.targetTotalRenderTime = (task.mode == OFFLINE_RENDER_MODE_EQUAL_TIME) ? task.renderTimeCompare : std::numeric_limits<float>::max();
		rightTask.targetTotalSamples    = (task.mode == OFFLINE_RENDER_MODE_EQUAL_SAMPLES) ? task.invocationsCompare : std::numeric_limits<uint32_t>::max();
		rightTask.args.config.seed += 0x146123;

		state = OFFLINE_RENDER_STATE_RIGHT;
		iec.cmdReset(cmdBuf, IEC_TARGET_RIGHT);
	}
	RenderTaskInternal *tasks[3]         = {&refTask, &leftTask, &rightTask};
	RenderTaskInternal &internalTask     = *tasks[state];
	std::string         internalTaskName = task.name + (state == OFFLINE_RENDER_STATE_REF ? "_ref" : state == OFFLINE_RENDER_STATE_LEFT ? "_left" :
	                                                                                                                                      "_right");
	bool                computeMSE       = state != OFFLINE_RENDER_STATE_REF;
	IECTarget           target           = state == OFFLINE_RENDER_STATE_REF ? IEC_TARGET_LEFT : IEC_TARGET_RIGHT;
	if (!internalTask.isComplete())
	{
		iec.cmdRun<TraceArgs>(cmdBuf, cmdTrace, internalTask.args, target, &internalTask.avgSampleTime,
		                      computeMSE ? IEC_FLAGS_NONE : IEC_RUN_NO_MSE);
		if (computeMSE && (internalTask.execCnt >= 10))        // hacky fix
		{
			internalTask.mse.push_back(iec.getMSE());        // e-6
		}
	}
	else
	{
		waitIdle();
		printVka("Render %s complete", internalTaskName.c_str());
		iec.cmdShow(cmdBuf, internalTask.result, VkRect2D_OP(task.resolution), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target, task.toneMappingArgs);
		Image png = createImage(pPool, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		iec.cmdShow(cmdBuf, png, VkRect2D_OP(task.resolution), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target, task.toneMappingArgs);

		iec.cmdReset(cmdBuf, IEC_TARGET_RIGHT);

		std::string taskDir;
		if (gvar_eval_use_custom_export_path.val.v_bool && std::filesystem::exists(gvar_eval_custom_export_path.val.v_char_array.data()))
		{
			taskDir = std::string(gvar_eval_custom_export_path.val.v_char_array.data()) + "/" + task.name;
		}
		else
		{
			taskDir = resultsPath + task.name;
		}
		std::filesystem::create_directories(taskDir);
		std::string internalTaskPath = taskDir + "/" + internalTaskName;
		ExportTask  exportTask{};
		exportTask.path         = internalTaskPath + ".exr";
		exportTask.pResource    = internalTask.result;
		exportTask.targetFormat = EXPORT_FORMAT_EXR;
		gState.exporter->cmdExport(cmdBuf, exportTask);
		exportTask.path         = internalTaskPath + ".png";
		exportTask.pResource    = png;
		exportTask.targetFormat = EXPORT_FORMAT_PNG;
		gState.exporter->cmdExport(cmdBuf, exportTask);

		if (state != OFFLINE_RENDER_STATE_REF)
		{
			// Store mse over time data as csv
			Buffer mseBuffer = createBuffer(pPool, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
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
			state = OFFLINE_RENDER_STATE_NEXT;
			// Write json
			json          j;
			std::ofstream o(taskDir + "/" + task.name + "_info.json");
			j["resolution"] = {task.resolution.width, task.resolution.height};

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
			//iec.garbageCollect();
		}
	}
}

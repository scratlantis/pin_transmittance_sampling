#include "OfflineRenderer.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include "ui.h"
#include <parse_args.h>

using json = nlohmann::json;

//const float referenceRMStepSize = 0.01;
GVar        gvar_eval_ref_rm_step_size{"Ref Step Size", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_EVALUATION_PARAMS, {0.001f, 0.1f}};
extern GVar gvar_show_pin_grid_size_coef;

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
	if (task.mode == OFFLINE_RENDER_MODE_EQUAL_TIME)
	{
		float              totalTime = task.renderTimeRef + task.renderTimeCompare * 2;
		// clang-format off
		float              progress  = 
			refTask.avgSampleTime * refTask.execCnt
			+ leftTask.avgSampleTime * leftTask.execCnt
			+ rightTask.avgSampleTime * rightTask.execCnt;
		// clang-format on
		return glm::clamp(progress / totalTime, 0.f, 1.f);
	}
	else if (task.mode == OFFLINE_RENDER_MODE_EQUAL_SAMPLES)
	{
		float totalTime = task.renderTimeRef + task.invocationsCompare * 200.0; // 100 ms per invocation (just for progress visualization, does not realy matter)
		// clang-format off
		float progress = 
			refTask.avgSampleTime * refTask.execCnt
			+ leftTask.execCnt * 100.0
			+ rightTask.execCnt * 100.0;
		// clang-format on
		return glm::clamp(progress / totalTime, 0.f, 1.f);
	}
	else
	{
		return 0.0;
	}
}


std::string getAbsolutePath(std::string basePath, std::string relativPath)
{
	std::string prefix = "../";
	basePath = basePath.substr(0, basePath.find_last_of("/"));
	while (relativPath.find(prefix) == 0)
	{
		relativPath = relativPath.substr(prefix.size());
		basePath    = basePath.substr(0, basePath.find_last_of("/"));
	}
	return basePath + "/" + relativPath;
}

GVar gvar_eval_custom_export_path{"Custom export path", std::string("none"), GVAR_FILE_INPUT, GUI_CAT_EVALUATION_PATH, std::vector<std::string>({ "", getAbsolutePath(resultsPath, customExportRelativePath) })};
GVar gvar_eval_use_custom_export_path{"Use custom export path", false, GVAR_BOOL, GUI_CAT_EVALUATION_PATH};

GVar gvar_eval_use_sub_sample{"Use subsample", false, GVAR_BOOL, GUI_CAT_EVALUATION_PARAMS};

extern GVar gvar_sample_eval_target;

bool OfflineRenderer::cmdRunTick(CmdBuffer cmdBuf)
{
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

		iec            = ImageEstimatorComparator(VK_FORMAT_R32G32B32A32_SFLOAT, task.resolution);
		iec.cmdReset(cmdBuf);
		refTask.execCnt        = 0;
		leftTask.execCnt       = 0;
		rightTask.execCnt      = 0;
		TraceArgs commonArgs{};
		commonArgs.resources.cmdLoadAll(cmdBuf, pPool, this->pTraceResourceCache);
		commonArgs.sceneParams = task.sceneParams;
		commonArgs.config = task.config;
		if (!gvar_eval_use_sub_sample.val.v_bool)
		{
			commonArgs.config.subSampleMode = 1;
		}

		refTask.reset();
		refTask.args = TraceArgs(pTraceResourceCache, pPool, &refTask.execCnt);
		refTask.args.update(commonArgs);
		refTask.args.config.rayMarchStepSize = gvar_eval_ref_rm_step_size.val.v_float;
		refTask.result                       = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		refTask.targetTotalRenderTime		= task.renderTimeRef;
		refTask.targetTotalSamples			= std::numeric_limits<uint32_t>::max();
		refTask.args.config.seed += 0x543419;
		//refTask.args.config.subSampleMode = 2;

		leftTask.reset();
		leftTask.args  = TraceArgs(pTraceResourceCache, pPool, &leftTask.execCnt);
		leftTask.args.update(commonArgs);
		leftTask.args.cvsArgs = task.cvsArgsLeft;
		leftTask.result                = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		leftTask.targetTotalRenderTime = (task.mode == OFFLINE_RENDER_MODE_EQUAL_TIME) ? task.renderTimeCompare : std::numeric_limits<float>::max();
		leftTask.targetTotalSamples    = (task.mode == OFFLINE_RENDER_MODE_EQUAL_SAMPLES) ? task.invocationsCompare : std::numeric_limits<uint32_t>::max();
		if (task.skipLeft)
		{
			leftTask.targetTotalSamples = 1;
		}
		leftTask.args.config.seed += 0x47283;

		rightTask.reset();
		rightTask.args = TraceArgs(pTraceResourceCache, pPool, &rightTask.execCnt);
		rightTask.args.update(commonArgs);
		rightTask.args.cvsArgs = task.cvsArgsRight;
		rightTask.result                = createImage(pPool, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		rightTask.targetTotalRenderTime = (task.mode == OFFLINE_RENDER_MODE_EQUAL_TIME) ? task.renderTimeCompare : std::numeric_limits<float>::max();
		rightTask.targetTotalSamples    = (task.mode == OFFLINE_RENDER_MODE_EQUAL_SAMPLES) ? task.invocationsCompare : std::numeric_limits<uint32_t>::max();
		rightTask.args.config.seed += 0x146123;

		if (gvar_sample_eval_target.val.v_uint == EVAL_TARGET_NO_REF)
		{
			refTask.targetTotalSamples = 1;
		}
		else if (gvar_sample_eval_target.val.v_uint == EVAL_TARGET_NO_COMP)
		{
			leftTask.targetTotalSamples  = 1;
			rightTask.targetTotalSamples = 1;
		}
		state = OFFLINE_RENDER_STATE_REF;
		iec.cmdReset(cmdBuf);
	}
	RenderTaskInternal* tasks[3] = {&refTask, &leftTask, &rightTask};
	RenderTaskInternal &internalTask = *tasks[state];
	std::string internalTaskName = task.name + (state == OFFLINE_RENDER_STATE_REF ? "_ref" : state == OFFLINE_RENDER_STATE_LEFT ? "_left" : "_right");
	bool computeMSE = state != OFFLINE_RENDER_STATE_REF;
	IECTarget target = state == OFFLINE_RENDER_STATE_REF ? IEC_TARGET_LEFT : IEC_TARGET_RIGHT;
	if (!internalTask.isComplete())
	{
		iec.cmdRun<TraceArgs>(cmdBuf, cmdTrace, internalTask.args, target, &internalTask.avgSampleTime,
		                      computeMSE ? IEC_FLAGS_NONE : IEC_RUN_NO_MSE);
		if (computeMSE)        // && (internalTask.execCnt >= 10)) //hacky fix
		{
			if (gvar_sample_eval_target.val.v_uint != EVAL_TARGET_NO_REF)
			{
				internalTask.mse.push_back(iec.getMSE());        // e-6
			}
			else
			{
				internalTask.mse.push_back(1.0);
			}
		}
		if (internalTask.avgSampleTime * internalTask.execCnt > 1000.0 && !internalTask.hasFastResult && state != OFFLINE_RENDER_STATE_REF)
		{
			internalTask.hasFastResult = true;
			Image png                  = createImage(pPool, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
			iec.cmdShow(cmdBuf, png, VkRect2D_OP(task.resolution), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target, task.toneMappingArgs);
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
			exportTask.path         = internalTaskPath + "_1smp.png";
			exportTask.pResource    = png;
			exportTask.targetFormat = EXPORT_FORMAT_PNG;
			gState.exporter->cmdExport(cmdBuf, exportTask);

			internalTask.execCnt = 0;
			cmdFill(cmdBuf, iec.localAccumulationTargetRight, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, glm::vec4(0.0));
			internalTask.mse.clear();
		}
	}
	else
	{
		waitIdle();
		printVka("Render %s complete", internalTaskName.c_str());
		iec.cmdShow(cmdBuf, internalTask.result, VkRect2D_OP(task.resolution), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target, {});
		Image png = createImage(pPool, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, task.resolution);
		iec.cmdShow(cmdBuf, png, VkRect2D_OP(task.resolution), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target, task.toneMappingArgs);

		iec.cmdReset(cmdBuf, IEC_TARGET_RIGHT);

		std::string taskDir;
		if (gvar_eval_use_custom_export_path.val.v_bool
			&& std::filesystem::exists(gvar_eval_custom_export_path.val.v_char_array.data()))
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
		exportTask.pResource = internalTask.result;
		exportTask.targetFormat = EXPORT_FORMAT_EXR;
		gState.exporter->cmdExport(cmdBuf, exportTask);
		exportTask.path         = internalTaskPath + ".png";
		exportTask.pResource    = png;
		exportTask.targetFormat = EXPORT_FORMAT_PNG;
		gState.exporter->cmdExport(cmdBuf, exportTask);

		if (state != OFFLINE_RENDER_STATE_REF)
		{
			// Store mse over time data as csv
			for (size_t i = 0; i < internalTask.mse.size();i++)
			{
				printVka("MSE[%d]: %f", i, internalTask.mse[i]);
			}
			Buffer     mseBuffer = createBuffer(pPool, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			cmdWriteCopy(cmdBuf, mseBuffer, internalTask.mse.data(), internalTask.mse.size() * sizeof(float));
			ExportTask exportTask{};
			exportTask.path                 = internalTaskPath + "_mse.csv";
			exportTask.pResource            = mseBuffer;        // iec.getMSEBuf()->getSubBuffer({0, sizeof(float) * internalTask.execCnt});
			exportTask.targetFormat         = EXPORT_FORMAT_CSV;
			exportTask.bufferInfo.format    = EXPORT_BUFFER_FORMAT_FLOAT;
			exportTask.bufferInfo.rowLength = internalTask.execCnt;        // internalTask.mse.size();
			gState.exporter->cmdExport(cmdBuf, exportTask);
			//exportTask.path                 = internalTaskPath + "_rmse.csv";
			//exportTask.pResource            = iec.getRMSEBuf()->getSubBuffer({0, sizeof(float) * internalTask.execCnt});
			//exportTask.targetFormat         = EXPORT_FORMAT_CSV;
			//exportTask.bufferInfo.format    = EXPORT_BUFFER_FORMAT_FLOAT;
			//exportTask.bufferInfo.rowLength = internalTask.execCnt;        // internalTask.mse.size();
			//gState.exporter->cmdExport(cmdBuf, exportTask);
		}

		state = static_cast<OfflineRenderState>(state + 1);
		if (state == OFFLINE_RENDER_STATE_LEFT && task.skipLeft)
		{
			//state = static_cast<OfflineRenderState>(state + 1);
		}

		if (state == OFFLINE_RENDER_STATE_IDLE)
		{
			// Write json
			json          j;
			std::ofstream o(taskDir + "/" + task.name + "_info.json");
			j["resolution"]     = {task.resolution.width, task.resolution.height};
			j["step_size"]  = refTask.args.config.subSampleMode * refTask.args.config.subSampleMode;

			j["ref_avg_sample_time"] = refTask.avgSampleTime;
			j["ref_sample_count"]    = refTask.execCnt;

			j["left_avg_sample_time"] = leftTask.avgSampleTime;
			j["left_sample_count"]    = leftTask.execCnt;

			j["right_avg_sample_time"] = rightTask.avgSampleTime;
			j["right_sample_count"]    = rightTask.execCnt;

			j["grid_size_coef"] = gvar_show_pin_grid_size_coef.val.v_float;

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



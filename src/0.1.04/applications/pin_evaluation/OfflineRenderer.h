#pragma once
#include "config.h"
#include "pt_interface.h"


struct OfflineRenderTask
{
	std::string      name;
	TracerConfig     argsRef;
	TraceSceneParams sceneParams;
	CVSArgs          cvsArgsLeft;
	CVSArgs          cvsArgsRight;
	TracerConfig     config;
	VkExtent2D       resolution;
	float            renderTimeRef;
	float            renderTimeCompare;
};

class OfflineRenderer
{
	struct RenderTaskInternal
	{
		TraceArgs args;
		uint32_t  execCnt;
		float     avgSampleTime;
		float     targetTotalRenderTime; // in ms
		std::vector<float> mse;
		Image     result;

		bool isComplete() const
		{
			return avgSampleTime * execCnt >= targetTotalRenderTime;
		}

		void reset()
		{
			execCnt = 0;
			avgSampleTime = 0;
			mse.clear();
		}
	};

	enum OfflineRenderState
	{
		OFFLINE_RENDER_STATE_REF = 0,
		OFFLINE_RENDER_STATE_LEFT = 1,
		OFFLINE_RENDER_STATE_RIGHT = 2,
		OFFLINE_RENDER_STATE_IDLE = 3,
	};

	ImageEstimatorComparator       iec;
	std::vector<OfflineRenderTask> taskQueue;
	RenderTaskInternal 		 refTask, leftTask, rightTask;
	TraceResources                 traceResources;
	IResourceCache                *pTraceResourceCache;
	IResourcePool 			      *pPool;
	OfflineRenderState             state;

  public:
	OfflineRenderer();
	uint32_t getTaskQueueSize();
	float    getTaskProgress();
	DELETE_COPY_CONSTRUCTORS(OfflineRenderer);
	bool cmdRunTick(CmdBuffer cmdBuf);
	void addTask(OfflineRenderTask task);
};


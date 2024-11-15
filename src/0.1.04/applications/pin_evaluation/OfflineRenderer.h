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
	ImageEstimatorComparator       iec;
	std::vector<OfflineRenderTask> taskQueue;
	bool                           newTask      = true;
	uint32_t                       execCntRef   = 0;
	uint32_t                       execCntLeft  = 0;
	uint32_t                       execCntRight = 0;
	TraceArgs                      argsRef, argsLeft, argsRight;
	TraceResources                 traceResources;
	IResourceCache                *pTraceResourceCache;
	IResourcePool 			      *pPool;

  public:
	OfflineRenderer();
	DELETE_COPY_CONSTRUCTORS(OfflineRenderer);
	bool cmdRunTick(CmdBuffer cmdBuf);
	void addTask(OfflineRenderTask task);
};


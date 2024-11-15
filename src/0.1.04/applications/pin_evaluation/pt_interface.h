#pragma once
#include "config.h"
#include "pt_debug.h"
#include "custom_volume_sampler_smd.h"
#include "pt_resources.h"

using namespace default_scene;

struct TraceSceneParams
{
	float    areaLightEmissionScale;
	float    envMapEmissionScale;
	bool     skipGeometry;
	CameraCI cameraCI;
};

struct TracerConfig
{
	float rayMarchStepSize;

	uint32_t minDepth;
	uint32_t seed;
	uint32_t firstRandomBounce;
	uint32_t subSampleMode;

	uint32_t sampleCount;
	uint32_t maxDepth;
	uint32_t force_ray_marched_distance_sampling;
	uint32_t force_ray_marched_transmittance_sampling_al;
	uint32_t force_ray_marched_transmittance_sampling_env_map;
};

struct TraceArgs
{
	TraceResources   resources;
	TraceSceneParams sceneParams;
	TracerConfig     config;
	CVSArgs          cvsArgs;

	bool           enableDebugging;
	TraceDebugArgs debugArgs;

	uint32_t       *pExecutionCounter = nullptr;
	IResourceCache *pTraceResourceCache;

	void updateFast(const TraceArgs& newArgs)
	{
		resources = newArgs.resources;
		sceneParams.areaLightEmissionScale = newArgs.sceneParams.areaLightEmissionScale;
		sceneParams.envMapEmissionScale = newArgs.sceneParams.envMapEmissionScale;
		sceneParams.cameraCI = newArgs.sceneParams.cameraCI;

		config.minDepth = newArgs.config.minDepth;
		config.seed = newArgs.config.seed;
		config.firstRandomBounce = newArgs.config.firstRandomBounce;
		config.subSampleMode = newArgs.config.subSampleMode;
	}

	void update(const TraceArgs& newArgs)
	{
		resources = newArgs.resources;
		sceneParams = newArgs.sceneParams;
		config = newArgs.config;

		cvsArgs.defaultUpdateMode = newArgs.cvsArgs.defaultUpdateMode;
		cvsArgs.updateRate = newArgs.cvsArgs.updateRate;
		cvsArgs.traceUpdateArgs = newArgs.cvsArgs.traceUpdateArgs;
		cvsArgs.rayMarchingCoefficient = newArgs.cvsArgs.rayMarchingCoefficient;
		cvsArgs.pinGridExtent = newArgs.cvsArgs.pinGridExtent;
		cvsArgs.pinArgs = newArgs.cvsArgs.pinArgs;

		enableDebugging = newArgs.enableDebugging;
		debugArgs = newArgs.debugArgs;
	}

	void init(IResourceCache *pTraceResourceCache, IResourcePool* pPool, uint32_t *pExecutionCounter)
	{
		this->pExecutionCounter   = pExecutionCounter;
		this->pTraceResourceCache = pTraceResourceCache;
		this->cvsArgs.pinGridBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	}

	TraceArgs() = default;
	~TraceArgs() = default;
	TraceArgs(IResourceCache *pTraceResourceCache, IResourcePool* pPool, uint32_t *pExecutionCounter)
	{
		init(pTraceResourceCache, pPool, pExecutionCounter);
	}


};

void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args);
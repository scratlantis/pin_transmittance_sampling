#include "config.h"
#include "pt_debug.h"
#include "custom_volume_sampler_smd.h"
#include "pt_resources.h"

using namespace default_scene;

struct TraceArgs
{
	TraceResources resources;
	CameraCI       cameraCI;

	// general config:

	// submodule
	float               rayMarchStepSize;

	// shader runtime
	float areaLightEmissionScale;
	float envMapEmissionScale;
	uint32_t minDepth;
	uint32_t seed;
	uint32_t firstRandomBounce;
	uint32_t subSampleMode;

	// shader compile time
	uint32_t sampleCount;
	uint32_t maxDepth;
	bool     skipGeometry;
	uint32_t force_ray_marched_distance_sampling;
	uint32_t force_ray_marched_transmittance_sampling_al;
	uint32_t force_ray_marched_transmittance_sampling_env_map;

	uint32_t* pExecutionCounter = nullptr;

	// custom medium sampler:
	CVSArgs 		   cvsArgs;
	// debug:
	bool                enableDebugging;
	TraceDebugArgs      debugArgs;

	IResourceCache*     pTraceResourceCache;
};

void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args);
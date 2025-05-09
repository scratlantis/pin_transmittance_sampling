#include "config.h"
#include "pt_debug.h"
#include "custom_volume_sampler_smd.h"

using namespace default_scene;

struct TraceArgs
{
	// default scene:
	CameraCI            cameraCI;
	USceneData          sceneData;

	// medium:
	Buffer              mediumInstanceBuffer;
	TLAS                mediumTlas;
	Image               mediumTexture;

	// general config:

	// submodule
	float               rayMarchStepSize;

	// shader runtime
	float areaLightEmissionScale;
	float envMapEmissionScale;

	// shader compile time
	uint32_t sampleCount;
	uint32_t maxDepth;
	uint32_t minDepth;
	uint32_t fixedSeed;
	uint32_t firstRandomBounce;
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
};

void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args);
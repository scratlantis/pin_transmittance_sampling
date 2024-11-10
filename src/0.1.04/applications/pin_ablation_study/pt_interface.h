#include "config.h"
#include "pt_debug.h"

using namespace default_scene;

struct TraceArgs
{
	// default scene
	CameraCI            cameraCI;
	USceneData          sceneData;

	// general config
	uint32_t            sampleCount;
	uint32_t            maxDepth;
	float               areaLightEmissionScale;
	float               envMapEmissionScale;

	// medium
	Buffer              mediumInstanceBuffer;
	TLAS                mediumTlas;
	Image               mediumTexture;
	float               rayMarchStepSize;

	// custom medium sampler
	
	// todo
	
	// debug
	bool                enableDebugging;
	TraceDebugArgs      debugArgs;
};

void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args);
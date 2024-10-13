#include "config.h"


struct TraceArgs
{
	uint32_t frameIdx;
	mat4     viewMat;
	mat4     projMat;

	USceneData sceneData;
	Buffer volumeInstanceBuffer;
	Image  volumeTexture;

};


void trace(Image target, TraceArgs args)
{
	ComputeCmd cmd(target->getExtent2D(), shaderPath + "pt.comp", {{"FORMAT1", getGLSLFormat(target->getFormat())}});


}
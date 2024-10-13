#include "config.h"
using namespace default_scene;

#include "shaders/interface_structs.glsl"

struct TraceArgs
{
	uint32_t   maxDepth;
	float      rayMarchStepSize;
	CameraCI   cameraCI;
	USceneData sceneData;
	Buffer     mediumInstanceBuffer;
	Image      mediumTexture;
};


void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args)
{
	Buffer camBuf, camInstBuf;
	std::hash<std::string> hasher;
	gState.dataCache->fetch(camBuf, hasher("cam_buf"));
	gState.dataCache->fetch(camBuf, hasher("cam_inst_buf"));
	cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, args.cameraCI);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	ComputeCmd cmd(target->getExtent2D(), shaderPath + "pt.comp", {{"FORMAT1", getGLSLFormat(target->getFormat())}});
	bindCamera(cmd, camBuf, camInstBuf);
	bindScene(cmd, &args.sceneData);
	bindScalarField(cmd, args.mediumTexture, args.rayMarchStepSize);
	cmd.startLocalBindings();
	cmd.pushDescriptor(args.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pipelineDef.shaderDef.args.push_back({"MAX_BOUNCES", args.maxDepth});
	cmd.exec(cmdBuf);
}
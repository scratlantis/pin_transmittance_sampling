#include "fast_draw_commands.h"
#include "definition_build_tools.h"

void FastDrawState::drawRect(VkaCommandBuffer cmdBuf, VkImage dst, glm::vec4 color, VkRect2D_OP area)
{
	/*DrawCmd drawCmdMainWindow{};
	drawCmdMainWindow.renderArea = appConfig.mainViewport;

	DrawCmd drawCmd = {};
	setDefaults(drawCmd.pipelineDef);

	SurfaceData surfaceData = {};
	surfaceData.vertexCount  = 3;
	drawCmd.model.surfaceBuffer
	drawCmd.instanceBuffers = {appData.sphereTransformBuf};
	addDescriptor(drawCmd, appData.camConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, appData.viewConstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, appData.guiVarBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addDescriptor(drawCmd, appData.cubeTransformBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addInput(drawCmd.pipelineDef, PosNormalVertex::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_VERTEX);
	addInput(drawCmd.pipelineDef, Transform::getVertexDataLayout(), VK_VERTEX_INPUT_RATE_INSTANCE);
	addDepthAttachment(drawCmd, depthImage, true, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	addColorAttachment(drawCmd, offscreenImage);
	createFramebuffer(drawCmd, framebufferCache);
	drawCmd.pipelineDef.rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;
	drawCmd.pipelineDef.rasterizationState.frontFace   = VK_FRONT_FACE_CLOCKWISE;
	drawCmd.pipelineDef.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	drawCmd.instanceCount                              = 1;
	addDescriptor(drawCmd, appData.gaussianBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/vert.vert", {});
	addShader(drawCmd.pipelineDef, newShaderPath + "transmittance/gaussian/exact.frag",
	          {{"GAUSSIAN_COUNT", std::to_string(appConfig.gaussianCount)}});
	vkaCmdDraw(cmdBuf, drawCmd);*/
}
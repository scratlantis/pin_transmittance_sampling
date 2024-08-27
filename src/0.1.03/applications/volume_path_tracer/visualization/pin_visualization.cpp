#include <vka/vka.h>
#include "config.h"
#include "pin_visualization.h"



GVar gvar_cursor_pos_x{"Pin Cursor X", 0.5f, GVAR_UNORM, VISUALIZATION_SETTINGS};
GVar gvar_cursor_pos_y{"Pin Cursor Y", 0.5f, GVAR_UNORM, VISUALIZATION_SETTINGS};
GVar gvar_cursor_pos_z{"Pin Cursor Z", 0.5f, GVAR_UNORM, VISUALIZATION_SETTINGS};

GVar gvar_cursor_dir_phi{"Pin Cursor phi", 0.0f, GVAR_FLOAT_RANGE, VISUALIZATION_SETTINGS, {0.0f, 1.f * glm::pi<float>()}};
GVar gvar_cursor_dir_theta{"Pin Cursor theta", 0.0f, GVAR_FLOAT_RANGE, VISUALIZATION_SETTINGS, {0.0f, 2.f * glm::pi<float>()}};


void PinStateManager::writeGridPinState(CmdBuffer cmdBuf)
{
	ComputeCmd cmd = ComputeCmd(
	    glm::uvec3(gvar_pin_grid_size.val.v_uint, gvar_pin_grid_size.val.v_uint, gvar_pin_grid_size.val.v_uint),
	    //1,
		shaderPath + "visualization/write_grid_pin_state.comp");
	struct PushStruct
	{
		uint32_t gridSize;
		uint32_t gridCellSize;
	} pc;
	pc.gridSize = gvar_pin_grid_size.val.v_uint;
	pc.gridCellSize = gvar_pin_count_per_grid_cell.val.v_uint;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.pushDescriptor(pinState, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pMedium->pinGrid, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.exec(cmdBuf);
}

void PinStateManager::writeCursorPinState(CmdBuffer cmdBuf)
{
	ComputeCmd cmd = ComputeCmd(
	    1,
	    shaderPath + "visualization/write_cursor_pin_state.comp");
	struct PushStruct
	{
		uint32_t  gridSize;
		uint32_t  pinCountPerGridCell;
		glm::vec2 cursorDir;
		glm::vec3 cursorPos;
	} pc;
	pc.cursorPos           = cursorPos;
	pc.cursorDir           = cursorDirection;
	pc.gridSize            = gvar_pin_grid_size.val.v_uint;
	pc.pinCountPerGridCell = gvar_pin_count_per_grid_cell.val.v_uint;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.pushDescriptor(pinState, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pMedium->pinGrid, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.exec(cmdBuf);

}


void PinStateManager::resetPinState(CmdBuffer cmdBuf)
{
	pinState->changeSize(gvar_pin_count.val.v_uint * sizeof(uint32_t));
	pinState->addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	pinState->recreate();
	setDebugMarker(pinState, "PinState");
	cmdFillBuffer(cmdBuf, pinState, 0);
}

void PinStateManager::update(CmdBuffer cmdBuf)
{
	if (!pinState)
	{
		pinState = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	}
	
	resetPinState(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	writeGridPinState(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	cursorPos = glm::vec3(gvar_cursor_pos_x.val.v_float, gvar_cursor_pos_y.val.v_float, gvar_cursor_pos_z.val.v_float);
	cursorDirection = glm::vec2(gvar_cursor_dir_phi.val.v_float, gvar_cursor_dir_theta.val.v_float);

	writeCursorPinState(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

}

bool PinStateManager::requiresUpdate()
{
	glm::vec3 newCursorPos = glm::vec3(gvar_cursor_pos_x.val.v_float, gvar_cursor_pos_y.val.v_float, gvar_cursor_pos_z.val.v_float);
	glm::vec2 newCursorDir = glm::vec2(gvar_cursor_dir_phi.val.v_float, gvar_cursor_dir_theta.val.v_float);

	return glm::distance(newCursorPos, cursorPos) > 0.001f || glm::distance(newCursorDir, cursorDirection) > 0.001f;
}


void cmdVisualizePins(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Buffer pinBuffer, Buffer pinState, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout)
{
	DrawCmd     drawCmd     = DrawCmd();
	DrawSurface drawSurface{};
	drawSurface.vertexBuffer = pinBuffer;
	drawSurface.count		= pinBuffer->getSize() / sizeof(glm::vec3);
	drawSurface.indexBuffer = nullptr;
	drawSurface.vertexLayout.formats = {VK_FORMAT_R32G32B32_SFLOAT};
	drawSurface.vertexLayout.offsets = {0};
	drawSurface.vertexLayout.stride = sizeof(glm::vec3);
	drawCmd.setGeometry(drawSurface);

	glm::mat4 projectionMat                         = glm::perspective(glm::radians(60.0f), (float) dst->getExtent().width / (float) dst->getExtent().height, 0.1f, 500.0f);
	glm::mat4 vp                                    = projectionMat * cam->getViewMatrix() * objToWorld;
	drawCmd.pushDescriptor(cmdBuf, pPool, &vp, sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
	drawCmd.pushDescriptor(pinState, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	VkImageLayout finalLayout = dstLayout == VK_IMAGE_LAYOUT_UNDEFINED ? dst->getLayout() : dstLayout;
	Image         depthBuffer = gState.depthBufferCache->fetch(dst->getExtent2D());
	drawCmd.renderArea.extent                       = dst->getExtent2D();
	drawCmd.pipelineDef.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	addShader(drawCmd.pipelineDef, shaderPath + "visualization/show_pins.vert");

	if (clearDepth)
	{
		depthBuffer->setClearValue(ClearValue(1.0f, 0));
	}


	struct PushStruct
	{
		float alpha_0;
		float alpha_1;
	} pc;
	pc.alpha_0 = 0.3 / std::sqrt(0.01 * gvar_pin_count.val.v_uint);
	pc.alpha_1 = 0.3 / std::sqrt(0.01 * gvar_pin_count.val.v_uint);
	drawCmd.pushConstant(&pc, sizeof(PushStruct), VK_SHADER_STAGE_FRAGMENT_BIT);

	DrawCmd drawOpaque = drawCmd;
	addShader(drawOpaque.pipelineDef, shaderPath + "visualization/show_pins.frag", {{"OPAQUE", ""}});
	drawOpaque.pushColorAttachment(dst, finalLayout, BlendOperation::alpha(), BlendOperation::alpha());
	drawOpaque.pushDepthAttachment(depthBuffer, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	drawOpaque.exec(cmdBuf);

	DrawCmd drawTransparent = drawCmd;
	addShader(drawTransparent.pipelineDef, shaderPath + "visualization/show_pins.frag");
	drawTransparent.pushColorAttachment(dst, finalLayout, BlendOperation::alpha(), BlendOperation::alpha());
	drawTransparent.pushDepthAttachment(depthBuffer, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	drawTransparent.exec(cmdBuf);
}



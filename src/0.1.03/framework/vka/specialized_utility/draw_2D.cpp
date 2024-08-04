#include "draw_2D.h"
#include <vka/advanced_utility/complex_command_types.h>
#include <vka/advanced_utility/complex_command_construction.h>
namespace vka
{
DrawCmd getCmdFill(Image dst, glm::vec4 color)
{
	VkRect2D_OP area = {0, 0, dst->getExtent2D().width, dst->getExtent2D().height};
	return getCmdDrawRect(dst, color, area);
}

DrawCmd getCmdDrawRect(Image dst, glm::vec4 color, VkRect2D_OP area)
{
	DrawCmd drawCmd = DrawCmd();
	drawCmd.setGeometry(DrawSurface::screenFillingTriangle());
	drawCmd.pushColorAttachment(dst);
	drawCmd.pushConstant(&color, sizeof(glm::vec4));
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill.frag");
	return drawCmd;
}
} // namespace vka
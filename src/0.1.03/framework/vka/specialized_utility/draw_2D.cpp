#include "draw_2D.h"
#include <vka/advanced_utility/complex_commands.h>
#include <glm/gtc/matrix_transform.hpp>
namespace vka
{

DrawCmd getCmdFill(Image dst, glm::vec4 color)
{
	return getCmdFill(dst, dst->getLayout(), color);
}
DrawCmd getCmdFill(Image dst, VkImageLayout targetLayout, glm::vec4 color)
{
	VkRect2D_OP area = {0, 0, dst->getExtent2D().width, dst->getExtent2D().height};
	return getCmdDrawRect(dst, targetLayout, color, area);
}

DrawCmd getCmdDrawRect(Image dst, VkImageLayout dstLayout, glm::vec4 color, VkRect2D_OP area)
{
	DrawCmd drawCmd = DrawCmd();
	drawCmd.setGeometry(DrawSurface::screenFillingTriangle());
	drawCmd.pushColorAttachment(dst, dstLayout);
	drawCmd.pushConstant(&color, sizeof(glm::vec4), VK_SHADER_STAGE_FRAGMENT_BIT);
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_color.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_color.frag");
	return drawCmd;
}

DrawCmd getCmdAdvancedCopy(Image src, Image dst, VkImageLayout dstLayout, VkRect2D_OP srcArea, VkRect2D_OP dstArea)
{
	DrawCmd drawCmd = DrawCmd();
	drawCmd.setGeometry(DrawSurface::screenFillingTriangle());
	drawCmd.pushColorAttachment(dst, dstLayout);
	drawCmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	Rect2D<float>     region     = VkRect2D_OP::relRegion(dstArea, srcArea);
	drawCmd.pushConstant(&region, sizeof(Rect2D<float>), VK_SHADER_STAGE_FRAGMENT_BIT);
	drawCmd.renderArea = dstArea;
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_texture.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_texture.frag");
	return drawCmd;
}

DrawCmd getCmdAdvancedCopy(Image src, Image dst, VkImageLayout dstLayout)
{
	return getCmdAdvancedCopy(src, dst, dstLayout, VkRect2D_OP(src->getExtent2D()), VkRect2D_OP(dst->getExtent2D()));
}

} // namespace vka
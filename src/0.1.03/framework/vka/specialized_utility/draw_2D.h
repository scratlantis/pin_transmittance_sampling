#pragma once
#include <vka/advanced_utility/complex_command_types.h>
namespace vka
{
DrawCmd getCmdFill(Image dst, glm::vec4 color);
DrawCmd getCmdDrawRect(Image dst, glm::vec4 color, VkRect2D_OP area);
}
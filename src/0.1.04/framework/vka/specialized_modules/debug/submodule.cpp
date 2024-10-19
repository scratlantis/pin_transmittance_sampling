#include "submodule.h"
#include <glm/gtx/string_cast.hpp>
namespace vka
{
namespace shader_debug
{
void bindInvocationSelection(ComputeCmd &cmd, Buffer selectionBuf)
{
	cmd.pushSubmodule(cVkaShaderModulePath + "debug/select_single_invocation.glsl");
	cmd.pushDescriptor(selectionBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}
void cmdSelectInvocation(CmdBuffer cmdBuf, Buffer selectionBuf, glm::vec2 selection)
{
	cmdSelectInvocation(cmdBuf, selectionBuf, glm::vec3(selection, 0.0));
}
void cmdSelectInvocation(CmdBuffer cmdBuf, Buffer selectionBuf, glm::vec3 selection)
{
	selectionBuf->addUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	selectionBuf->changeSize(sizeof(glm::vec3));
	selectionBuf->recreate();
	cmdWriteCopy(cmdBuf, selectionBuf, &selection, sizeof(glm::vec3));
}

}        // namespace shader_debug
}        // namespace vka
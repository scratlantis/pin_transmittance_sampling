#include "VkaImGuiWrapper.h"
#include <vka/state_objects/global_var.h>
using namespace vka;
void guiConfigDefault();

namespace gvar_gui
{
void buildGui(VkRect2D_OP viewport);
void addGVar(GVar *gv);
void addGVars(GVar_Cat category);
}		// namespace gvar_gui

namespace shader_console_gui
{
void buildGui(VkRect2D_OP viewport);
}        // namespace shader_console_gui
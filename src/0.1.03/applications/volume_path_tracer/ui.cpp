#include "ui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>



void buildTitleBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Config"))
		{
			if (ImGui::MenuItem("Save", "STRG+S", &gvar_save_config.val.v_bool))
			{
				std::string name          = gvar_select_config.set.list[gvar_select_config.val.v_uint];
				name                      = name.substr(0, name.find_last_of("."));
				gvar_save_config_name.val = GVar_Val(name);
			}
			if (ImGui::BeginMenu("Save As"))
			{
				gvar_gui_v2::addGVar(&gvar_save_config_name);
				gvar_gui_v2::addGVar(&gvar_save_config);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Reload", "STRG+Z", &gvar_reload_config.val.v_bool)){}
			if (ImGui::BeginMenu("Load"))
			{
				gvar_gui_v2::addGVar(&gvar_select_config);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Geometry"))
		{
			gvar_gui_v2::addGVar(&gvar_model);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Env Map"))
		{
			gvar_gui_v2::addGVar(&gvar_env_map);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void resetEvents()
{
	for (auto &gv : gVars)
	{
		if (gv->type == GVAR_EVENT)
		{
			gv->val.v_bool = false;
		}
	}
}
void buildGui()
{
	// ToDo build title bar
	resetEvents();
	if (!gState.guiRendered)
	{
		gState.imguiWrapper->newFrame();
		gState.guiRendered = true;
	}
	//ImGui::ShowDemoWindow();

	
	setGuiDimensions(leftGuiDimensions);
	ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);
	buildTitleBar();
	gvar_gui_v2::buildGui(gVars, {"Path tracing", "Medium", "Noise", "Pins", "Visualization", "Metrics"});
	ImGui::End();


	shader_console_gui::buildGui(getScissorRect(viewDimensions));

}
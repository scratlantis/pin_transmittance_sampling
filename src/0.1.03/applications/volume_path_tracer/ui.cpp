#include "ui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

GuiConfig guiConf;

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

void showPlots()
{
	if (guiConf.showPlots)
	{
		{
			ImVec2      uv_min     = ImVec2(0.0f, 0.0f);                    // Top-left
			ImVec2      uv_max     = ImVec2(1.0f, 1.0f);                    // Lower-right
			ImVec4      tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // No tint
			ImVec4      border_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec2      size       = ImVec2(guiConf.plot1->getExtent2D().width, guiConf.plot1->getExtent2D().height);
			ImTextureID texID      = gState.imguiTextureIDCache->fetch(guiConf.plot1);
			ImGui::Image(texID, size, uv_min, uv_max, tint_col, border_col);
		}
		{
			ImVec2      uv_min     = ImVec2(0.0f, 0.0f);                    // Top-left
			ImVec2      uv_max     = ImVec2(1.0f, 1.0f);                    // Lower-right
			ImVec4      tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // No tint
			ImVec4      border_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec2      size       = ImVec2(guiConf.plot2->getExtent2D().width, guiConf.plot2->getExtent2D().height);
			ImTextureID texID      = gState.imguiTextureIDCache->fetch(guiConf.plot2);
			ImGui::Image(texID, size, uv_min, uv_max, tint_col, border_col);
		}
		{
			ImVec2      uv_min     = ImVec2(0.0f, 0.0f);                    // Top-left
			ImVec2      uv_max     = ImVec2(1.0f, 1.0f);                    // Lower-right
			ImVec4      tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // No tint
			ImVec4      border_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec2      size       = ImVec2(guiConf.plot3->getExtent2D().width, guiConf.plot3->getExtent2D().height);
			ImTextureID texID      = gState.imguiTextureIDCache->fetch(guiConf.plot3);
			ImGui::Image(texID, size, uv_min, uv_max, tint_col, border_col);
		}
		{
			ImVec2      uv_min     = ImVec2(0.0f, 0.0f);                    // Top-left
			ImVec2      uv_max     = ImVec2(1.0f, 1.0f);                    // Lower-right
			ImVec4      tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // No tint
			ImVec4      border_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec2      size       = ImVec2(guiConf.plot4->getExtent2D().width, guiConf.plot4->getExtent2D().height);
			ImTextureID texID      = gState.imguiTextureIDCache->fetch(guiConf.plot4);
			ImGui::Image(texID, size, uv_min, uv_max, tint_col, border_col);
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
	ImGui::ShowDemoWindow();

	
	setGuiDimensions(leftGuiDimensions);
	ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);
	buildTitleBar();
	gvar_gui_v2::buildGui(gVars, {"Path tracing", "Medium", "Noise", "Pins", "Visualization", "Metrics"});

	//if (guiConf.showPlots)
	//{
	//	ImVec2      uv_min     = ImVec2(0.0f, 0.0f);                    // Top-left
	//	ImVec2      uv_max     = ImVec2(1.0f, 1.0f);                    // Lower-right
	//	ImVec4      tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // No tint
	//	ImVec4      border_col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
	//	ImVec2      size       = ImVec2(guiConf.plot1->getExtent2D().width, guiConf.plot1->getExtent2D().height);
	//	ImTextureID texID      = gState.imguiTextureIDCache->fetch(guiConf.plot1);
	//	ImGui::Image(texID, size, uv_min, uv_max, tint_col, border_col);
	//}
	ImGui::End();

	setGuiDimensions(bottomGuiDimensions);
	ImGui::Begin("Plots", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
	showPlots();
	ImGui::End();
	shader_console_gui::buildGui(getScissorRect(viewDimensions));

}
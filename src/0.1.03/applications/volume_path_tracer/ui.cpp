#include "ui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

//namespace ImGui
//{

//bool CustomBeginMenuBar(float height)
//{
//		ImGuiWindow *window = GetCurrentWindow();
//		if (window->SkipItems)
//			return false;
//		if (!(window->Flags & ImGuiWindowFlags_MenuBar))
//			return false;
//
//		IM_ASSERT(!window->DC.MenuBarAppending);
//		BeginGroup();        // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
//		PushID("##menubar");
//
//		// We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
//		// We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area, which looks particularly glitchy.
//		ImRect bar_rect = window->MenuBarRect();
//	    bar_rect.Max.y  = height;
//		ImRect clip_rect(IM_ROUND(bar_rect.Min.x + window->WindowBorderSize), IM_ROUND(bar_rect.Min.y + window->WindowBorderSize), IM_ROUND(ImMax(bar_rect.Min.x, bar_rect.Max.x - ImMax(window->WindowRounding, window->WindowBorderSize))), IM_ROUND(bar_rect.Max.y));
//		clip_rect.ClipWith(window->OuterRectClipped);
//		PushClipRect(clip_rect.Min, clip_rect.Max, false);
//
//		// We overwrite CursorMaxPos because BeginGroup sets it to CursorPos (essentially the .EmitItem hack in EndMenuBar() would need something analogous here, maybe a BeginGroupEx() with flags).
//		window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->DC.MenuBarOffset.x, bar_rect.Min.y + window->DC.MenuBarOffset.y);
//		window->DC.LayoutType                          = ImGuiLayoutType_Horizontal;
//		window->DC.NavLayerCurrent                     = ImGuiNavLayer_Menu;
//		window->DC.MenuBarAppending                    = true;
//		AlignTextToFramePadding();
//		return true;
//	}
//}



void buildTitleBar()
{
	/*float height = getScissorRect(topGuiDimensions).extent.height;*/
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Config"))
		{
			if (ImGui::MenuItem("Save", "CRTL+S", &gvar_save_config.val.v_bool))
			{
			}
			if (ImGui::BeginMenu("Save As"))
			{
				gvar_gui_v2::addGVar(&gvar_save_config_name);
				gvar_gui_v2::addGVar(&gvar_save_config);
			}
			if (ImGui::MenuItem("Load"))
			{
				gvar_gui_v2::addGVar(&gvar_select_config);
			}
			ImGui::EndMenu();
			// ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
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
	/*setGuiDimensions({0,0,1,1});*/
	ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);
		buildTitleBar();
	    //VkRect2D_OP viewport = getScissorRect(addMarginToRect(leftGuiDimensions, viewMargin));
	    //ImGui::SetNextWindowPos({(float) viewport.offset.x, (float) viewport.offset.y});
	    //ImGui::BeginChild("Options", {(float) viewport.extent.width, (float) viewport.extent.height});
			gvar_gui_v2::buildGui(gVars, {"General", "Noise Function", "Pins", "Visualization", "Metrics"});
		//	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
		//ImGui::EndChild();
	ImGui::End();

	//ImGui::Begin("Gvar", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	//gvar_gui_v2::buildGui(gVars, {"General", "Noise Function", "Pins", "Visualization", "Metrics"});
	//ImGui::End();


	shader_console_gui::buildGui(getScissorRect(viewDimensions));

}
#include "misc_gui.h"
#include <vka/state_objects/global_var.h>
using namespace vka;

void guiConfigDefault()
{
	// Init stuff
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io         = ImGui::GetIO();
	float    sizePixels = 16;
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Roboto-Medium.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Cousine-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/DroidSans.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Karla-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyClean.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyTiny.ttf", sizePixels);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	ImGui::StyleColorsDark();
}

namespace gvar_gui
{
void addGVar(GVar *gv)
{
	std::stringstream ss;
	switch (gv->type)
	{
		case GVAR_EVENT:
			gv->val.v_bool = ImGui::Button(gv->path.c_str());
			break;
		case GVAR_BOOL:
			ImGui::Checkbox(gv->path.c_str(), &gv->val.v_bool);
			break;
		case GVAR_FLOAT:
			ImGui::InputScalar(gv->path.c_str(), ImGuiDataType_Float, &gv->val.v_float);
			break;
		case GVAR_UNORM:
			ImGui::SliderFloat(gv->path.c_str(), &gv->val.v_float, 0.0f, 1.0f);
			break;
		case GVAR_UINT:
			ImGui::InputScalar(gv->path.c_str(), ImGuiDataType_U32, &gv->val.v_uint);
			break;
		case GVAR_INT:
			ImGui::InputInt(gv->path.c_str(), &gv->val.v_int);
			break;
		case GVAR_VEC3:
			ImGui::InputFloat3(gv->path.c_str(), gv->val.v_vec3);
			break;
		case GVAR_DISPLAY_VALUE:
			ImGui::Text(gv->path.c_str(), gv->val.v_float);
			break;
		case GVAR_ENUM:
			for (size_t i = 0; i < gv->enumVal.size(); i++)
			{
				ss << gv->enumVal[i] << '\0';
			}
			ImGui::Combo(gv->path.c_str(), &gv->val.v_int, ss.str().c_str(), 5);
			break;
		default:
			break;
	}
}

void addGVars(GVar_Cat category)
{
	for (uint32_t i = 0; i < gVars.size(); i++)
	{
		if (gVars[i]->cat == category)
		{
			ImGui::PushItemWidth(ImGui::GetWindowSize().x * 0.3);
			addGVar(gVars[i]);
		}
	}
}

void buildGui(VkRect2D_OP viewport)
{
	ImGui::SetNextWindowPos({(float)viewport.offset.x, (float)viewport.offset.y});
	ImGui::SetNextWindowSize({(float) viewport.extent.width, (float) viewport.extent.height});
	ImGui::Begin("Gvar", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::CollapsingHeader("Runtime Settings"))
	{
		addGVars(GVAR_RUNTIME_SETTINGS);
	}
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::CollapsingHeader("Load Settings"))
	{
		addGVars(GVAR_LOAD_SETTINGS);
	}
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	if (ImGui::CollapsingHeader("Load Settings"))
	{
		addGVars(GVAR_WINDOW_SETTINGS);
	}
	ImGui::End();
}
}        // namespace gvar_gui

namespace shader_console_gui
{
void buildGui(VkRect2D_OP viewport)
{
	ImGui::SetNextWindowPos({(float) viewport.offset.x, (float) viewport.offset.y});
	ImGui::SetNextWindowSize({(float) viewport.extent.width, (float) viewport.extent.height});
	ImGui::Begin("ShaderLog", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImGui::TextWrapped(gShaderLog.c_str());
	ImGui::End();
}
}        // namespace shader_console_guis
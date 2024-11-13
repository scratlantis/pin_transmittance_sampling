#include "ui.h"

extern GVar gvar_pt_plot_write_total_contribution;
extern GVar gvar_pt_plot_write_indirect_dir;
extern GVar gvar_pt_plot_write_indirect_t;
extern GVar gvar_pt_plot_write_indirect_weight;

extern GVar gvar_timing_left;
extern GVar gvar_timing_right;

uint32_t getPlotCount()
{
	return gvar_pt_plot_write_total_contribution.val.bool32()
		+ gvar_pt_plot_write_indirect_dir.val.bool32()
		+ gvar_pt_plot_write_indirect_t.val.bool32()
		+ gvar_pt_plot_write_indirect_weight.val.bool32();
}

std::vector<bool> buildGui(CmdBuffer cmdBuf, ImageEstimatorComparator* pIEC)
{
	std::hash<std::string> h;
	beginGui("Settings", leftGuiDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
	//// GVars
	std::vector<bool> changed = GVar::addAllToGui<GuiCatergories>();

	//// Historgams
	if (ImGui::CollapsingHeader("Histograms"))
	{
		void                  *pHist, *pHistData, *pHistCount;
		bool                   dataAquired =
		    gState.feedbackDataCache->fetchHostData(pHist, h("hist")) && gState.feedbackDataCache->fetchHostData(pHistData, h("histData")) && gState.feedbackDataCache->fetchHostData(pHistCount, h("histCount"));
		if (dataAquired)
		{
			uint32_t histCount = *static_cast<uint32_t *>(pHistCount);
			pHistData = reinterpret_cast<unsigned char *>(pHistData) + getLeftHistogramOffset();
			addPlots<shader_plot::GLSLHistogram>(static_cast<shader_plot::GLSLHistogram *>(pHist), histCount, pHistData);
		}
	}

	//// Plots
	if (ImGui::CollapsingHeader("Plots"))
	{
		void                  *pHist, *pHistData, *pHistCount;
		bool                   histDataAquired =
		    gState.feedbackDataCache->fetchHostData(pHist, h("hist"))
			&& gState.feedbackDataCache->fetchHostData(pHistData, h("histData"))
			&& gState.feedbackDataCache->fetchHostData(pHistCount, h("histCount"));

		void *ptPlot;
		bool  ptPlotDataAquired = gState.feedbackDataCache->fetchHostData(ptPlot, h("ptPlot"));

		if (histDataAquired && ptPlotDataAquired)
		{
			// clang-format off
			ImPlotSubplotFlags flags = ImPlotSubplotFlags_NoLegend
				| ImPlotSubplotFlags_ColMajor;
			// clang-format on
			if (getPlotCount() > 0 && ImPlot::BeginSubplots("##Plots", getPlotCount(), 2, ImVec2(-1, 800), flags))
			{
				render_plot_family<pt_plot::GLSLPtPlot>{}(*static_cast<pt_plot::GLSLPtPlot *>(ptPlot), pHist, pHistData);
				unsigned char *pHistData2 = reinterpret_cast<unsigned char *>(pHistData) + getLeftHistogramOffset();
				render_plot_family<pt_plot::GLSLPtPlot>{}(*static_cast<pt_plot::GLSLPtPlot *>(ptPlot), pHist, pHistData2);
				ImPlot::EndSubplots();
			}
		}
	}
	endGui();
	//// Shader Log Gui
	if (gState.shaderLog.size() > 0)
	{
		beginGui("Shader Log", viewDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
		ImGui::TextWrapped(gState.shaderLog.c_str());
		endGui();
	}

	beginGui("TestTop", topGuiDimensions, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar, nullptr);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			ImGui::TextWrapped("ToDo");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	float coef = gvar_timing_left.val.v_float / (gvar_timing_right.val.v_float + gvar_timing_left.val.v_float);
	ImGui::PushItemWidth(0.98*getScissorRect(topGuiDimensions).extent.width);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, {1.0, 0.0, 0.0, 1.0});
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, {0.0, 1.0, 0.0, 1.0});
	ImGui::ProgressBar(coef, ImVec2(0.0f, 0.0f));
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	endGui();

	beginGui("TestBottom", bottomGuiDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar, nullptr);

	ImVec2            uv_min     = ImVec2(0.0f, 0.0f);                    // Top-left
	ImVec2            uv_max     = ImVec2(1.0f, 1.0f);                    // Lower-right
	ImVec4            tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // No tint
	ImVec4            border_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	VkImageCreateInfo imgCI      = ImageCreateInfo_Default(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VkExtent2D{256, 128}, VK_FORMAT_R8G8B8A8_UNORM);
	Image             diffImg;
	gState.imageCache->fetch(cmdBuf, diffImg, h("dif_img"), imgCI, VK_IMAGE_LAYOUT_GENERAL, vec4(0.0));
	pIEC->showDiff(cmdBuf, diffImg);
	cmdTransitionLayout(cmdBuf, diffImg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkRect2D_OP localExtent = getScissorRect(bottomGuiDimensions);
	ImVec2      size        = ImVec2(localExtent.extent.width * 0.48, localExtent.extent.height * 0.9);
	ImTextureID texID = gState.imguiTextureIDCache->fetch(diffImg);
	ImGui::Image(texID, size, uv_min, uv_max, tint_col, border_col);
	ImGui::SameLine();
	ImGui::BeginChild("child", ImVec2(ImGui::GetContentRegionAvail()), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);


	uint32_t frames = pIEC->getMSEDataSize();
	uint32_t leftFrames = frames * coef;
	uint32_t rightFrames = frames - leftFrames;
	float    totalTime   = gvar_timing_left.val.v_float * leftFrames + gvar_timing_right.val.v_float * rightFrames;
	float avgFrameTimeMS = totalTime / frames;
	uint32_t offset         = frames * 0.5;
	if (ImPlot::BeginPlot("Avg squared diff", size))
	{
		ImPlot::SetupAxes("x", "y", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
		ImPlot::PlotShaded("f(x)", pIEC->getMSEData() + offset, frames - offset, 0.0, avgFrameTimeMS * 0.001, offset * avgFrameTimeMS * 0.001, 0, 0);
		ImPlot::EndPlot();
	}



	ImGui::EndChild();


	endGui();

	//ImPlot::ShowDemoWindow();
	//ImGui::ShowDemoWindow();
	return changed;
}
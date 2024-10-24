#include "ui.h"

GVar gvar_perlin_frequency{"Perlin frequency", 4.f, GVAR_FLOAT_RANGE, GUI_CAT_NOISE, {1.f, 10.f}};
GVar gvar_pt_plot_write_total_contribution{"Write total contribution", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_dir{"Write indirect direction", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_t{"Write indirect t", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_weight{"Write indirect weight", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_bounce{"Select Bounce", 0, GVAR_UINT_RANGE, GUI_CAT_PT_PLOT, {0, 5}};
GVar gvar_mse{"MSE: %.8f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_left{"Timing Left: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_right{"Timing Left: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};


GVar gvar_bitmask_propability{"Bitmask propability", 0.f, GVAR_UNORM, GUI_CAT_DEBUG};
GVar gvar_bitmask_iterations{"Bitmask iterations", 5U, GVAR_UINT_RANGE, GUI_CAT_DEBUG, {1U, 10U}};



uint32_t getPlotCount()
{
	return gvar_pt_plot_write_total_contribution.val.bool32()
		+ gvar_pt_plot_write_indirect_dir.val.bool32()
		+ gvar_pt_plot_write_indirect_t.val.bool32()
		+ gvar_pt_plot_write_indirect_weight.val.bool32();
}

std::vector<bool> buildGui()
{
	beginGui("Settings", leftGuiDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
	//// GVars
	std::vector<bool> changed = GVar::addAllToGui<GuiCatergories>();

	//// Historgams
	if (ImGui::CollapsingHeader("Histograms"))
	{
		void                  *pHist, *pHistData, *pHistCount;
		std::hash<std::string> h;
		bool                   dataAquired =
		    gState.feedbackDataCache->fetchHostData(pHist, h("hist")) && gState.feedbackDataCache->fetchHostData(pHistData, h("histData")) && gState.feedbackDataCache->fetchHostData(pHistCount, h("histCount"));
		if (dataAquired)
		{
			uint32_t histCount = *static_cast<uint32_t *>(pHistCount);
			addPlots<shader_plot::GLSLHistogram>(static_cast<shader_plot::GLSLHistogram *>(pHist), histCount, pHistData);
		}
	}

	//// Plots
	if (ImGui::CollapsingHeader("Plots"))
	{
		void                  *pHist, *pHistData, *pHistCount;
		std::hash<std::string> h;
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
				| ImPlotSubplotFlags_ColMajor
				//| ImPlotSubplotFlags_LinkCols
				| ImPlotSubplotFlags_LinkRows
				//| ImPlotSubplotFlags_LinkAllX
				//| ImPlotSubplotFlags_LinkAllY
				;
			// clang-format on
			if (getPlotCount() > 0 && ImPlot::BeginSubplots("##Plots", getPlotCount(), 2, ImVec2(-1, 800), flags))
			{
				render_plot_family<pt_plot::GLSLPtPlot>{}(*static_cast<pt_plot::GLSLPtPlot *>(ptPlot), pHist, pHistData);
				unsigned char *pHistData2 = reinterpret_cast<unsigned char *>(pHistData) + maxHistValueCount/2;
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
	//ImPlot::ShowDemoWindow();
	//ImGui::ShowDemoWindow();
	return changed;
}
#include "ui.h"



std::vector<bool> buildGui()
{
	beginGui("Settings", leftGuiDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
	//// GVars
	std::vector<bool> changed = GVar::addAllToGui<GuiCatergories>();
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
			render_plot_family<pt_plot::GLSLPtPlot>{}(*static_cast<pt_plot::GLSLPtPlot *>(ptPlot), pHist, pHistData);
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
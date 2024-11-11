#include "ui.h"

GVar gvar_perlin_frequency{"Perlin frequency", 4.f, GVAR_FLOAT_RANGE, GUI_CAT_NOISE, {1.f, 10.f}};
GVar gvar_perlin_scale{"Perlin scale", 1000.f, GVAR_FLOAT_RANGE, GUI_CAT_NOISE, {0.f, 4000.f}};

GVar gvar_pt_plot_write_total_contribution{"Write total contribution", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_dir{"Write indirect direction", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_t{"Write indirect t", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_write_indirect_weight{"Write indirect weight", false, GVAR_BOOL, GUI_CAT_PT_PLOT};
GVar gvar_pt_plot_bounce{"Select Bounce", 0, GVAR_UINT_RANGE, GUI_CAT_PT_PLOT, {0, 5}};
GVar gvar_mse{"MSE: %.8f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_left{"Timing Left: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};
GVar gvar_timing_right{"Timing Right: %.4f", 0.f, GVAR_DISPLAY_VALUE, GUI_CAT_METRICS};


GVar gvar_bitmask_propability{"Bitmask propability", 0.f, GVAR_UNORM, GUI_CAT_DEBUG};
GVar gvar_bitmask_iterations{"Bitmask iterations", 5U, GVAR_UINT_RANGE, GUI_CAT_DEBUG, {1U, 10U}};
GVar gvar_enable_debuging{"Enable Debuging", false, GVAR_BOOL, GUI_CAT_DEBUG};

GVar gvar_pin_pos_grid_size{"Pin Pos Grid Size", 10U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 64U}};
GVar gvar_pin_dir_grid_size{"Pin Dir Grid Size", 8U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 256U}};
GVar gvar_pin_ray_march_step_size{"Pin Ray March Step Size", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.01f, 1.f}};
GVar gvar_pin_write_pin_step_size{"Pin Write Pin Step Size", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PINS, {0.01f, 1.f}};
GVar gvar_pin_update_rate{"Pin Update Rate", 1000U, GVAR_UINT_RANGE, GUI_CAT_PINS, {0U, 100000U}};
GVar gvar_pin_disable_bit_mask_distance_sampling{"Disable bitmask distance sampling", false, GVAR_BOOL, GUI_CAT_PINS};
GVar gvar_pin_disable_bit_mask_transmittance_sampling{"Disable bitmask transmittance sampling", false, GVAR_BOOL, GUI_CAT_PINS};

GVar gvar_pin_bit_mask_size{"Bit mask size", 1U, GVAR_UINT_RANGE, GUI_CAT_PINS, {1U, 4U}};
GVar gvar_pin_update_all{"Update all bitmasks per frame", false, GVAR_BOOL, GUI_CAT_PINS};


GVar gvar_ray_march_step_size{"Ray March Step Size", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PATH_TRACING, {0.01f, 1.f}};
GVar gvar_bounce_count{"Bounce Count", 5U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {1U, 16U}};
GVar gvar_min_bounce{"Min Bounce", 0U, GVAR_UINT_RANGE, GUI_CAT_PATH_TRACING, {0U, 16U}};
GVar gvar_skip_geometry{"Skip geometry", false, GVAR_BOOL, GUI_CAT_PATH_TRACING};
GVar gvar_emission_scale_al{"Area light emission scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_PATH_TRACING, {0.0f, 100000.f}};
GVar gvar_emission_scale_env_map{"Env map emission scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_PATH_TRACING, {0.0f, 10.f}};

GVar gvar_fixed_seed{"Fixed seed", 0U, GVAR_UINT_RANGE, GUI_CAT_PT_RANDOMIZATION, {0U, 10000U}};
GVar gvar_first_random_bounce{"First random bounce", 0U, GVAR_UINT_RANGE, GUI_CAT_PT_RANDOMIZATION, {0U, 16U}};


GVar gvar_tone_mapping_enable{"Tone mapping", true, GVAR_BOOL, GUI_CAT_TONE_MAPPING};
GVar gvar_tone_mapping_whitepoint{"Tone mapping whitepoint", 4.f, GVAR_FLOAT_RANGE, GUI_CAT_TONE_MAPPING, {1.f, 10.f}};
GVar gvar_tone_mapping_exposure{"Tone mapping exposure", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_TONE_MAPPING, {0.001f, 1.f}};



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
			pHistData = reinterpret_cast<unsigned char *>(pHistData) + getLeftHistogramOffset();
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
				//| ImPlotSubplotFlags_LinkRows
				//| ImPlotSubplotFlags_LinkAllX
				//| ImPlotSubplotFlags_LinkAllY
				;
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
	//ImPlot::ShowDemoWindow();
	ImGui::ShowDemoWindow();
	return changed;
}
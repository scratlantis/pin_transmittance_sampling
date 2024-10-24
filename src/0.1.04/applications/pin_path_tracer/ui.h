#pragma once
#include "config.h"


enum GuiCatergories
{
	GUI_CAT_NOISE,
	GUI_CAT_PT_PLOT,
	GUI_CAT_METRICS,
	GUI_CAT_DEBUG,
};
const std::vector<std::string> GuiCatergories_names_ = {"Noise", "Plot Settings", "Metrics", "Debug"};
ENUM_ADD_STRING_VALUES(GuiCatergories, GuiCatergories_names_)
ENUM_ADD_ITERATOR(GuiCatergories, GUI_CAT_NOISE, GUI_CAT_DEBUG)

std::vector<bool> buildGui();



extern GVar gvar_perlin_frequency;
extern GVar gvar_pt_plot_write_total_contribution;
extern GVar gvar_pt_plot_write_indirect_dir;
extern GVar gvar_pt_plot_write_indirect_t;
extern GVar gvar_pt_plot_write_indirect_weight;
extern GVar gvar_pt_plot_bounce;
extern GVar gvar_mse;
extern GVar gvar_timing_left;
extern GVar gvar_timing_right;

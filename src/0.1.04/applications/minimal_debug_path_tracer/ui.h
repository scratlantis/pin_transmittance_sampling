#pragma once
#include "config.h"

const float viewMargin = 0.001;
const Rect2D<float> viewDimensions    = {0.3, viewMargin, 0.7 - 1.0 * viewMargin, 1.0 - 2.0 * viewMargin};
const Rect2D<float> leftGuiDimensions = {0.f + viewMargin, viewDimensions.y, viewDimensions.x - 2.0 * viewMargin, viewDimensions.height};
const Rect2D<float> topGuiDimensions  = {0, 0, 0.5, viewDimensions.y};


enum GuiCatergories
{
	GUI_CAT_NOISE,
	GUI_CAT_PT_PLOT,
};
const std::vector<std::string> GuiCatergories_names_ = {"Noise", "Plot Settings"};
ENUM_ADD_STRING_VALUES(GuiCatergories, GuiCatergories_names_)
ENUM_ADD_ITERATOR(GuiCatergories, GUI_CAT_NOISE, GUI_CAT_PT_PLOT)

std::vector<bool> buildGui();




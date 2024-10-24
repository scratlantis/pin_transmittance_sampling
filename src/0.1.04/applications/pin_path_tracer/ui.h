#pragma once
#include "config.h"


enum GuiCatergories
{
	GUI_CAT_NOISE,
	GUI_CAT_PINS,
	GUI_CAT_DEBUG,
	GUI_CAT_METRICS,
	GUI_CAT_PT_PLOT,
};
const std::vector<std::string> GuiCatergories_names_ = {"Noise","Pins","Debug","Metrics","Plot Settings"};
ENUM_ADD_STRING_VALUES(GuiCatergories, GuiCatergories_names_)
ENUM_ADD_ITERATOR(GuiCatergories, GUI_CAT_NOISE, GUI_CAT_DEBUG)

std::vector<bool> buildGui();








#pragma once
#include "config.h"


enum GuiCatergories
{
	GUI_CAT_SCENE,
	GUI_CAT_MEDIUM,
	GUI_CAT_PATH_TRACING,
	GUI_CAT_TONE_MAPPING,
	GUI_CAT_PINS,
	GUI_CAT_RENDER_MODE,
	GUI_CAT_METRICS,
	GUI_CAT_DEBUG
};
const std::vector<std::string> GuiCatergories_names_ = {"Scene", "Medium", "Path Tracing", "Tone Mapping", "Pins", "Render Mode", "Metrics", "Debug"};
ENUM_ADD_STRING_VALUES(GuiCatergories, GuiCatergories_names_)
ENUM_ADD_ITERATOR(GuiCatergories, GUI_CAT_SCENE, GUI_CAT_DEBUG)

std::vector<bool> buildGui();








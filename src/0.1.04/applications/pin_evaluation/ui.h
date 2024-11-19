#pragma once
#include "config.h"

enum GuiGroups
{
	GUI_GROUP_FILE = 0,
	GUI_GROUP_SCENE = 1,
	GUI_GROUP_SETTINGS = 2,
	GUI_GROUP_EVALUATION = 3,
	GUI_GROUP_DEBUG = 4,
};

enum GuiGroupMenuBar
{
	GUI_CAT_MENU_BAR = 0x000F0,
};

const uint32_t GUI_INDEX_MASK = 0x0000F;
const uint32_t GUI_CAT_SHIFT = 4;


const std::vector<std::string> GuiGroupMenuBar_names = {"Mode"};
MASKED_ENUM_ADD_STRING_VALUES(GuiGroupMenuBar, GuiGroupMenuBar_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupMenuBar, GUI_CAT_MENU_BAR, GUI_CAT_MENU_BAR)

enum GuiGroupFile
{
	GUI_CAT_FILE_LOAD = 0x00000,
	GUI_CAT_FILE_SAVE = 0x00001,
};
const std::vector<std::string> GuiGroupFile_names = {"Load", "Save"};
MASKED_ENUM_ADD_STRING_VALUES(GuiGroupFile, GuiGroupFile_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupFile, GUI_CAT_FILE_LOAD, GUI_CAT_FILE_SAVE)

enum GuiGroupScene {
	GUI_CAT_SCENE_GENERAL          = 0x00010,
	GUI_CAT_SCENE_MEDIUM           = 0x00011,
	GUI_CAT_SCENE_MEDIUM_INSTANCES = 0x00012,
	GUI_CAT_SCENE_PARAMS           = 0x00013,
	GUI_CAT_SCENE_TRANSFORMS       = 0x00014,
};
const std::vector<std::string> GuiGroupScene_names = {"General", "Medium", "Medium Instances", "Params", "Transforms"};
MASKED_ENUM_ADD_STRING_VALUES(GuiGroupScene, GuiGroupScene_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupScene, GUI_CAT_SCENE_GENERAL, GUI_CAT_SCENE_TRANSFORMS)



enum GuiGroupSettings
{
	GUI_CAT_PATH_TRACING = 0x00020,
	GUI_CAT_TONE_MAPPING = 0x00021,
	GUI_CAT_PINS = 0x00022,
	GUI_CAT_RENDER_MODE = 0x00023,
};
const std::vector<std::string> GuiGroupSettings_names = {"Path Tracing", "Tone Mapping", "Pins", "Render Mode"};
MASKED_ENUM_ADD_STRING_VALUES(GuiGroupSettings, GuiGroupSettings_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupSettings, GUI_CAT_PATH_TRACING, GUI_CAT_RENDER_MODE)

enum GuiGroupEvaluation
{
	GUI_CAT_EVALUATION = 0x00030,
	GUI_CAT_METRICS = 0x00031,
};
const std::vector<std::string> GuiGroupEvaluation_names = {"Evaluation", "Metrics"};
MASKED_ENUM_ADD_STRING_VALUES(GuiGroupEvaluation, GuiGroupEvaluation_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupEvaluation, GUI_CAT_EVALUATION, GUI_CAT_METRICS)

enum GuiGroupDebug
{
	GUI_CAT_DEBUG = 0x00040,
};
const std::vector<std::string> GuiGroupDebug_names = {"Debug"};
MASKED_ENUM_ADD_STRING_VALUES(GuiGroupDebug, GuiGroupDebug_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupDebug, GUI_CAT_DEBUG, GUI_CAT_DEBUG)


std::vector<bool> buildGui(CmdBuffer cmdBuf, ImageEstimatorComparator *pIEC);








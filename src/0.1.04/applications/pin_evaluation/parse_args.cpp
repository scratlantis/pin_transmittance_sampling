#pragma once
#include "config.h"
#include "misc.h"
#include "ui.h"
#include "parse_args.h"


//https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
char *getCmdOption(char **begin, char **end, const std::string &option)
{
	char **itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

bool cmdOptionExists(char **begin, char **end, const std::string &option)
{
	return std::find(begin, end, option) != end;
}

GVar gvar_sample_eval_target{"Eval Target", 0U, GVAR_ENUM, GUI_CAT_EVALUATION, std::vector<std::string>({"Full", "No ref", "No comp"}), GVAR_FLAGS_NO_LOAD};


bool parse_args(int argc, char **argv, AppArgs& args)
{
	uint argsFound = 0;
	if (cmdOptionExists(argv, argv + argc, "-s"))
	{
		argsFound++;
		args.settingsPath = getCmdOption(argv, argv + argc, "-s");
	}
	if (cmdOptionExists(argv, argv + argc, "-p"))
	{
		argsFound++;
		args.renderParamsPath = getCmdOption(argv, argv + argc, "-p");
	}
	if (cmdOptionExists(argv, argv + argc, "-o"))
	{
		argsFound++;
		args.outputDir = getCmdOption(argv, argv + argc, "-o");
	}
	if (cmdOptionExists(argv, argv + argc, "-n"))
	{
		argsFound++;
		args.outputName = getCmdOption(argv, argv + argc, "-n");
	}
	if (cmdOptionExists(argv, argv + argc, "-b"))
	{
		argsFound++;
		args.baseSettingsPath = getCmdOption(argv, argv + argc, "-b");
	}
	else
	{
		args.baseSettingsPath = "";
	}
	if (cmdOptionExists(argv, argv + argc, "-o"))
	{
		argsFound++;
		args.overridePath = getCmdOption(argv, argv + argc, "-override");
	}
	else
	{
		args.overridePath = "";
	}
	if (cmdOptionExists(argv, argv + argc, "-noref"))
	{
		argsFound++;
		gvar_sample_eval_target.val.v_uint = EVAL_TARGET_NO_REF;
	}
	if (cmdOptionExists(argv, argv + argc, "-nocomp"))
	{
		argsFound++;
		gvar_sample_eval_target.val.v_uint = EVAL_TARGET_NO_COMP;
	}
	return argsFound >= 4;
}

extern GVar gvar_eval_custom_export_path;
extern GVar gvar_eval_name;
extern GVar gvar_eval_use_custom_export_path;

void loadArgs(AppArgs args)
{
	if (!args.baseSettingsPath.empty() && std::filesystem::exists(args.baseSettingsPath))
	{
		GVar::loadAll(args.baseSettingsPath);
	}
	if (std::filesystem::exists(args.settingsPath))
	{
		GVar::loadAll(args.settingsPath);
	}

	if (std::filesystem::exists(args.renderParamsPath))
	{
		std::vector<GVar *> vars = GVar::filterSortID(GVar::getAll(), GUI_CAT_EVALUATION_PARAMS);
		GVar::load(vars, args.renderParamsPath);
	}
	if (!args.overridePath.empty() && std::filesystem::exists(args.overridePath))
	{
		GVar::loadAll(args.overridePath);
		printVka("Override successful");
	}

	if (dirExists(args.outputDir.c_str()))
	{
		gvar_eval_custom_export_path.val.v_char_array = std::vector<char>(args.outputDir.begin(), args.outputDir.end());
		gvar_eval_custom_export_path.val.v_char_array.push_back('\0');
		gvar_eval_use_custom_export_path.val.v_bool = true;
	}

	if (!args.outputName.empty())
	{
		gvar_eval_name.val.v_char_array = std::vector<char>(args.outputName.begin(), args.outputName.end());
		gvar_eval_name.val.v_char_array.push_back('\0');
	}
}

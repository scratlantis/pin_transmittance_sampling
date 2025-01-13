#pragma once

struct AppArgs
{
	std::string settingsPath;            // -s
	std::string renderParamsPath;        // -p
	std::string outputDir;               // -o
	std::string outputName;              // -n
	std::string baseSettingsPath;        // -b
};

bool parse_args(int argc, char **argv, AppArgs &args);
void loadArgs(AppArgs args);
enum EvalTarget
{
	EVAL_TARGET_FULL,
	EVAL_TARGET_NO_REF,
	EVAL_TARGET_NO_COMP
};
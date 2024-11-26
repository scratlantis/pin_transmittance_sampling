#pragma once

struct AppArgs
{
	std::string settingsPath;            // -s
	std::string renderParamsPath;        // -p
	std::string outputDir;               // -o
	std::string outputName;              // -n
};

bool parse_args(int argc, char **argv, AppArgs &args);
void loadArgs(AppArgs args);
#pragma once
#include "config.h"


enum class CVSUpdateMode
{
	ALL,
	TRACE
};

struct TraceUpdateArgs
{
	uint32_t rayCount;
	float   writePinStepSize;
};

struct PinGridExtent
{
	uint positionGridSize;
	uint directionGridSize;
};

enum class PinType : uint32_t
{
	NONE = 0U,
	V1 = 1U,
	V2 = 2U,
	V3 = 3U,
};

enum class PinSampleMode : uint32_t
{
	UNQUANTISED = 0U,
	QUANTISED = 1U,
	PRECOMPUTED = 2U,
};

struct PinArgs
{
	PinType       type              = PinType::NONE;
	PinSampleMode sampleMode        = PinSampleMode::UNQUANTISED;
	uint32_t      bitMaskIterations = 5; // iterations for sample bitmask generation
	float         jitterPos         = 0.0f;
	float         jitterDir         = 0.0f;
};

struct CVSArgs
{
	CVSUpdateMode   defaultUpdateMode = CVSUpdateMode::TRACE;
	uint32_t        updateRate        = 1;        // every n-th frame
	TraceUpdateArgs traceUpdateArgs;
	float           rayMarchingCoefficient;        // pin ray marche step size relative to the ray march step size in trace args
	PinGridExtent   pinGridExtent;
	uint32_t        frameID;
	PinArgs			pinArgs;
};

struct CVSData
{
	Buffer pinGridBuffer;
};

struct TraceArgs;
CVSData cmdPrepareCVSData(CmdBuffer cmdBuf, const TraceArgs &args);
void    bindCVSModule(ComputeCmd &cmd, const CVSData &data, const CVSArgs &args);
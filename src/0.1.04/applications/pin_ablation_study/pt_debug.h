#pragma once
#include "config.h"

struct TraceDebugArgs
{
	vec2                   pixelPos;
	bool                   enablePlot;
	uint32_t               maxPlotCount;
	uint32_t               maxPlotValueCount;
	bool                   enableHistogram;
	bool                   resetHistogram;
	uint32_t               histogramDataOffset;
	uint32_t               maxHistCount;
	uint32_t               maxHistValueCount;
	bool                   enablePtPlot;
	pt_plot::PtPlotOptions ptPlotOptions;
};

struct TraceDebugData
{
	// Plot resources:
	Buffer selectionBuf, plotBuf, plotDataBuf, plotCountBuf;
	// Histogram resources:
	Buffer histBuf, histDataBuf, histCountBuf;
	// Shader state resources:
	Buffer indirectBounceBuf, directRayBuf, stateBuf;
	// Pt Plot resources:
	Buffer ptPlotOptionsBuf, ptPlotBuf;
};

struct TraceArgs;
TraceDebugData cmdPrepareDebugData(CmdBuffer cmdBuf, TraceArgs args);
void           bindDebugModules(ComputeCmd &cmd, TraceDebugData data, TraceDebugArgs args);
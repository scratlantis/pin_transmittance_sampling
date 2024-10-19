#ifndef Y_LIST_PLOT_SMD_H
#define Y_LIST_PLOT_SMD_H
#extension GL_EXT_debug_printf : enable
//debugPrintfEXT("Test");
#include "../../lib/compute_shader_base.glsl"
#include "../../lib/plot/interface_structs.glsl"

layout(binding = Y_LIST_PLOT_SMD_BINDING_OFFSET) buffer PLOTS
{
	GLSLYListPlot plots[];
};
layout(binding = Y_LIST_PLOT_SMD_BINDING_OFFSET + 1) buffer PLOT_DATA
{
	float plot_data[];
};
layout(binding = Y_LIST_PLOT_SMD_BINDING_OFFSET + 2) buffer PLOT_COUNT
{
	uint plot_count;
};

void initPlot()
{
	debugPrintfEXT("Init plot!");
	plot_count = 1;
	plots[0].invocationID = invocationID();
	plots[0].plotID = plot_count;
	plots[0].offset = 0;
	plots[0].count = 0;
	plots[0].stride = 1.0;
}

void nextPlot()
{
	uint currentOffset =  plots[plot_count - 1].offset + plots[plot_count - 1].count;
	plot_count++;
	plots[plot_count - 1].invocationID = invocationID();
	plots[plot_count - 1].plotID = plot_count;
	plots[plot_count - 1].offset = currentOffset;
	plots[plot_count - 1].count = 0;
	plots[plot_count - 1].stride = 1.0;
}

void setStride(float stride)
{
	plots[plot_count - 1].stride = stride;
}

void addValue(float value)
{
	plot_data[plots[plot_count - 1].offset + plots[plot_count - 1].count] = value;
	plots[plot_count - 1].count++;
}

#endif
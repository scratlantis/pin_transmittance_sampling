

#ifndef INSPECTION_H
#define INSPECTION_H


struct ShaderState
{
	uint bounce;
	// Last pin
	int pinID;
	float pinTransmittance;
	float pinSampleDistance;

	int currentPlot;

	uint plotValOffset;
	uint plotStepOffset;
};

const uint PLOT_INTERPOLATE_NONE = 0;
const uint PLOT_INTERPOLATE_LINEAR = 1;


const uint PLOT_TYPE_NONE = 0;
const uint PLOT_TYPE_UNIFORM = 1; // set value, add value
const uint PLOT_TYPE_UNIFORM_ACC = 2; // set value, add value
const uint PLOT_TYPE_DYNAMIC = 3; // next value(v)
const uint PLOT_TYPE_DYNAMIC_STEP = 4; // next value(v,s)


// Plot types: uniform, uniform_acc, dynamic, dynamic_step





const uint ERROR_GENERAL = 1;

struct Error
{
	uint errorType;
};

struct Plot1D
{
	uint type;
	uint interpolate;
	uint sampleCount;

	bool useMaxVal;
	float maxVal;

	uint valOffset;
	uint valCount;

	int stepOffset; 
};

layout(binding = PT_INSPECTION_BINDING_OFFSET) buffer PLOT_BUF { Plot1D plotBuf[] };
layout(binding = PT_INSPECTION_BINDING_OFFSET + 1) buffer PLOT_VAL_BUF { float plotValBuf[] };
layout(binding = PT_INSPECTION_BINDING_OFFSET + 2) buffer PLOT_STEP_BUF { float plotStepBuf[] };
layout(binding = PT_INSPECTION_BINDING_OFFSET + 4) buffer PLOT_COUNT { uint plotCountBuf };
layout(binding = PT_INSPECTION_BINDING_OFFSET + 3) buffer ERROR_BUF { Error errorBuf };
shared ShaderState ss;

void new_plot_uniform(uint valCount)
{
	Plot1D plot;
	plot.type = PLOT_TYPE_UNIFORM;
	plot.interpolate = PLOT_INTERPOLATE_NONE;
	plot.accumulate = false;
	plot.maxVal = 0.0;
	plot.valCount = valCount;
	plot.valOffset = ss.plotValOffset;

	ss.plotValOffset += valCount;
	plotBuf[++ss.currentPlot] = plot;

	plotCountBuf = ss.currentPlot + 1;
}

void plot_set_value(uint idx, float val)
{
	Plot1D plot = plotBuf[ss.currentPlot];
	if(plot.type != PLOT_TYPE_UNIFORM)
	{
		errorBuf.errorType = ERROR_GENERAL;
		return;
	}
	if(idx < plot.valCount)
	{
		plotValBuf[plot.valOffset + idx] = val;
	}
	else
	{
		errorBuf.errorType = ERROR_GENERAL;
	}
}

void new_plot_dynamic_step






#endif

#ifndef PT_PROFILING_H
#define PT_PROFILING_H

layout(binding = PT_PROFILING_BINDING_OFFSET) writeonly buffer LINE_SEGMENTS{ GLSLLineSegment line_segment_buf[]; };
layout(binding = PT_PROFILING_BINDING_OFFSET + 1) writeonly buffer PLOT_BUF{ float plotBuf[]; };
layout(binding = PT_PROFILING_BINDING_OFFSET + 2) writeonly buffer FLUX_BUF{ float fluxBuf[]; };



void writePinFlux(uint pinIdx, float weight)
{
#ifdef WRITE_PIN_FLUX
	if(pinIdx != 0)
	{
		atomicAdd(fluxBuf[pinIdx], weight);
	}
#endif
}

void writePinFlux(uint pinIdx, vec3 weight)
{
#ifdef WRITE_PIN_FLUX
	if(pinIdx != 0)
	{
		atomicAdd(fluxBuf[pinIdx], weight.r + weight.g + weight.b);
	}
#endif
}


void writePinPlot(vec3 origin, vec3 direction, float segmentLenght, uint seed, uint bounce)
{
#ifdef PLOT_TRANSMITTANCE
	uvec2 gID = gl_GlobalInvocationID.xy;
	uvec2 mousePixel = uvec2(params.cursorPos * vec2(frame.width, frame.height));
	if((gID == mousePixel) && (params.pathSampling == 1) && (bounce == 0))
	{
		vec2 uv;
		GLSLPinGridEntry pinGridEntry = selectPin(origin, direction, segmentLenght, seed);
		float distanceSum = 0.0;
		uint offset = PLOT_OFFSET_PINS;
		float stepSize = segmentLenght/float(PLOT_POINTS);
		plotBuf[offset] = float(PLOT_POINTS);
		for(uint i = 0; i < PLOT_POINTS; i++)
		{
			vec3 destination = origin + direction * stepSize * float(1 + i);
			float transmittance = samplePin(origin, destination, pinGridEntry, uv);
			plotBuf[offset + 2 + 2 * i + 0] = stepSize;
			plotBuf[offset + 2 + 2 * i + 1] = transmittance;
		}
		plotBuf[offset + 1] = segmentLenght;
	}
#endif
}

void writeScalarFieldPlot(vec3 origin, vec3 direction, float segmentLenght, uint seed, uint bounce)
{
#ifdef PLOT_TRANSMITTANCE
	uvec2 gID = gl_GlobalInvocationID.xy;
	uvec2 mousePixel = uvec2(params.cursorPos * vec2(frame.width, frame.height));
	if((gID == mousePixel) && (params.pathSampling == 1) && (bounce == 0))
	{
		uint offset = PLOT_OFFSET_SCALAR_FIELD;
		float stepSize = segmentLenght/float(PLOT_POINTS);
		plotBuf[offset] = float(PLOT_POINTS);
		for(uint i = 0; i < PLOT_POINTS; i++)
		{
			float localTMax = stepSize * float(1 + i);
			uint localSeed = seed;
			float transmittance = rayMarcheMediumTransmittance(origin, direction, localTMax, localSeed);
			//transmittance = float(1 + i)/float(PLOT_POINTS);
			plotBuf[offset + 2 + 2 * i + 0] = stepSize;
			plotBuf[offset + 2 + 2 * i + 1] = transmittance;
		}
		plotBuf[offset + 1] = segmentLenght;
	}
#endif
}



#define LINE_SEGMENT_TYPE_INDIRECT 0
#define LINE_SEGMENT_TYPE_DIRECT_AL 1
#define LINE_SEGMENT_TYPE_DIRECT_ENV 2
#define LINE_SEGMENT_TYPE_PIN_AL 3
#define LINE_SEGMENT_TYPE_PIN_ENV 4
vec3 lineSegmentColor(uint type)
{
	switch (type) {
		case LINE_SEGMENT_TYPE_INDIRECT:
			return vec3(0.0, 0.5, 0.0);
		case LINE_SEGMENT_TYPE_DIRECT_AL:
			return vec3(0.0, 0.0, 1.0);
		case LINE_SEGMENT_TYPE_DIRECT_ENV:
			return vec3(0.5, 0.5, 1.0);
		case LINE_SEGMENT_TYPE_PIN_AL:
			return vec3(1.0, 0.0, 0.0);
		case LINE_SEGMENT_TYPE_PIN_ENV:
			return vec3(1.0, 0.5, 0.5);
		default:
		return vec3(0.0, 0.0, 0.0);
}
}

void writeLineSegment(vec3 start, vec3 end, uint bounce, uint type)
{
#ifdef WRITE_LINE_SEGMENTS
	uvec2 gID = gl_GlobalInvocationID.xy;
	uvec2 mousePixel = uvec2(params.cursorPos * vec2(frame.width, frame.height));
	if(gID == mousePixel
	&& 
	//( params.pathSampling == 1 || (params.leftMB == BUTTON_PRESS_EVENT && params.controlKEY == BUTTON_PRESSED) ))
	( params.pathSampling == 1))
	{
		GLSLLineSegment line;
		line.start = start;
		line.end = end;
		line.color = lineSegmentColor(type);
		line.cullMask = 0xF0;
		uint offset = LINE_SEGMENTS_PER_BOUNCE * bounce + type;
		line_segment_buf[offset] = line;
	}
#endif
}
#endif

#ifndef PIN_SMD_H
#define PIN_SMD_H

#include "interface_structs.glsl"

layout(binding = PIN_SMD_BINDING_OFFSET) readonly buffer PIN_GRID
{
	GLSLPinCacheEntry pin_grid[];
};
#include "pin_common.glsl"


layout(constant_id = PIN_SMD_SPEC_CONST_OFFSET) const uint scBitMaskIterations = 5;

float pinSampleDistance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	uint pinIdx = pin_cache_offset(origin,direction);
	GLSLPinCacheEntry pin = pin_grid[pinIdx];
	if(pin.maxColProb == MAX_FLOAT)
	{
		SI_printf("Invalid pin\n");
		return TMAX;
	}
	uint sampleMask = randomBitMask(pin.maxColProb, scBitMaskIterations, seed);

	bool inverseDir;
	inverseDir = cartesianToSpherical(direction).y > PI * 0.5;
	vec3 start, end;
	unitCubeIntersection(origin, direction, start, end);
	float sampleCoef;
	uint pinMask = pin.mask[0];
	if(inverseDir)
	{
		sampleCoef = distance(origin, end);
		pinMask = bitfieldReverse(pinMask);
	}
	else
	{
		sampleCoef = distance(origin, start);
	}
	sampleCoef /= distance(start, end);

	uint sampleOffset = uint(clamp(sampleCoef, 0.0, 0.9999) * 32);
	pinMask = pinMask << sampleOffset;
	uint finalMask = sampleMask & pinMask;
	uint sampledDiscreteDist = findMSB(finalMask);
	if(sampledDiscreteDist > 32 - sampleOffset)
	{
		return TMAX;
	}
	float sampledDist = sampledDiscreteDist / 32.0 * distance(start, end);
	if(sampledDist > maxLength)
	{
		return TMAX;
	}
	return sampledDist;
};



#endif
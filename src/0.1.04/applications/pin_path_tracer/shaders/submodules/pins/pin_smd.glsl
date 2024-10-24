#ifndef PIN_SMD_H
#define PIN_SMD_H

layout(binding = PIN_SMD_BINDING_OFFSET) readonly buffer PIN_GRID
{
	GLSLPinCacheEntry pin_grid;
};

#include "pin_common.glsl"


#define BIT_MASK_ITERATIONS = 5;
float pinSampleDistance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	uint pinIdx = pin_cache_offset(pos,dir);
	GLSLPinCacheEntry pin = pin_grid[pinIdx];
	uint sampleMask = randomBitMask(maxColProb, BIT_MASK_ITERATIONS, seed);

	bool inverseDir;
	inverseDir = cartesianToSpherical(dir).y > PI * 0.5;
	vec3 pinStart, pinEnd;
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
	uint pinMask = pinMask << sampleOffset;
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
}


#endif
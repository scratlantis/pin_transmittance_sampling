#ifndef PIN_SMD_H
#define PIN_SMD_H

#include "interface_structs.glsl"

layout(binding = PIN_SMD_BINDING_OFFSET) readonly buffer PIN_GRID
{
	GLSLPinCacheEntry pin_grid[];
};
#include "pin_common.glsl"
layout(constant_id = PIN_SMD_SPEC_CONST_OFFSET) const uint scBitMaskIterations = 5;

#ifndef DISABLE_BITMASK_SAMPLING

#if 0
float pinSampleDistance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	uint pinIdx = pin_cache_offset(origin,direction);
	GLSLPinCacheEntry pin = pin_grid[pinIdx];
	if(pin.maxColProb == MAX_FLOAT)
	{
		return TMAX;
	}
	uint sampleMask = randomBitMask(pin.maxColProb, scBitMaskIterations, seed);

	bool inverseDir = cartesianToSpherical(direction).y > PI * 0.5;
	vec3 start, end;
	unitCubeIntersection(origin, direction, start, end);


	uint pinMask = pin.mask[0];


	if(inverseDir)
	{
		pinMask = bitfieldReverse(pinMask);
		//debugPrintfEXT( "Was Here! %d\n", seed);
	}
	float sampleCoef = distance(origin, start) / distance(start, end);

	uint sampleOffset = uint(clamp(sampleCoef, 0.0, 0.9999) * 32);
	float delta = clamp(sampleCoef, 0.0, 0.9999) * 32.0*-floor(clamp(sampleCoef, 0.0, 0.9999) * 32.0);

	pinMask = pinMask << sampleOffset;
	uint finalMask = sampleMask & pinMask;
	uint sampledDiscreteDist = findMSB(finalMask);
	if(sampledDiscreteDist == -1)
	{
		return TMAX;
	}
	sampledDiscreteDist = 31 - sampledDiscreteDist;
	if(sampledDiscreteDist > 32 - sampleOffset)
	{
		return TMAX;
	}
	float sampledDist = ((sampledDiscreteDist + unormNext(seed)) / 32.0) * distance(start, end);
	
	if(sampledDist > maxLength)
	{
		return TMAX;
	}
	return sampledDist;
};
#else
float pinSampleDistance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	vec3 jitteredOrigin = origin + (vec3(0.5) - random3D(seed)) * 1.0 / PIN_POS_GRID_SIZE;
	vec3 jitteredDirection = direction + (vec3(0.5) - random3D(seed)) * 1.0 / PIN_DIR_GRID_SIZE;
	jitteredOrigin = clamp(jitteredOrigin, vec3(0.0), vec3(1.0));
	jitteredDirection = normalize(jitteredDirection);

	uint pinIdx = pin_cache_offset(jitteredOrigin,jitteredDirection);

	GLSLPinCacheEntry pin = pin_grid[pinIdx];
	if(pin.maxColProb == MAX_FLOAT)
	{
		return TMAX;
	}

	bool inverseDir = cartesianToSpherical(direction).y > PI * 0.5;
	vec3 start, end;
	unitCubeIntersection(origin, direction, start, end);

	float sampleCoef = distance(origin, start) / distance(start, end);
	uint sampleOffset = uint(clamp(sampleCoef, 0.0, 0.9999) * 32 * PIN_MASK_SIZE);
	float perBitDistance = distance(start, end) / float(PIN_MASK_SIZE * 32);

	uint maskOffset = sampleOffset / 32;
	uint currentBitOffset = sampleOffset%32;

	int intersectionBit = -1;

	for(uint i = maskOffset; i < PIN_MASK_SIZE; i++)
	{
		uint pinMask;
		if(inverseDir)
		{
			pinMask = bitfieldReverse(pin.mask[PIN_MASK_SIZE-(i+1)]);
		}
		else
		{
			pinMask = pin.mask[i];
		}
		pinMask = pinMask << currentBitOffset;
		uint sampleMask = randomBitMask(pin.maxColProb, scBitMaskIterations, seed);
		uint finalMask = sampleMask & pinMask;
		uint sampledDiscreteDist = 31 - findMSB(finalMask);
		if(sampledDiscreteDist != 32)
		{
			intersectionBit = int(i * 32 + currentBitOffset + sampledDiscreteDist);
			break;
		}
		currentBitOffset = 0;
	}
	if(intersectionBit == -1)
	{
		return TMAX;
	}
	intersectionBit -= int(sampleOffset);
	float sampledDist = float(intersectionBit + unormNext(seed)) * perBitDistance;
	//SI_printf("Sampled distance: %f.3\n", sampledDist);
	if(sampledDist > maxLength)
	{
		return TMAX;
	}
	return sampledDist;
};

float pinSampleTransmittance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	vec3 jitteredOrigin = origin + (vec3(0.5) - random3D(seed)) * 1.0 / PIN_POS_GRID_SIZE;
	vec3 jitteredDirection = direction + (vec3(0.5) - random3D(seed)) * 1.0 / PIN_DIR_GRID_SIZE;
	jitteredOrigin = clamp(jitteredOrigin, vec3(0.0), vec3(1.0));
	jitteredDirection = normalize(jitteredDirection);

	uint pinIdx = pin_cache_offset(jitteredOrigin,jitteredDirection);

	GLSLPinCacheEntry pin = pin_grid[pinIdx];
	if(pin.maxColProb == MAX_FLOAT)
	{
		return 1.0;
	}

	bool inverseDir = cartesianToSpherical(direction).y > PI * 0.5;
	vec3 start, end;
	unitCubeIntersection(origin, direction, start, end);

	float sampleCoef = distance(origin, start) / distance(start, end);
	uint sampleOffset = uint(clamp(sampleCoef, 0.0, 0.9999) * 32 * PIN_MASK_SIZE);
	float perBitDistance = distance(start, end) / float(PIN_MASK_SIZE * 32);

	uint maskOffset = sampleOffset / 32;
	uint currentBitOffset = sampleOffset%32;

	int intersectionBit = -1;

	uint oneBitCnt = 0;

	for(uint i = maskOffset; i < PIN_MASK_SIZE; i++)
	{
		uint pinMask;
		if(inverseDir)
		{
			pinMask = bitfieldReverse(pin.mask[PIN_MASK_SIZE-(i+1)]);
		}
		else
		{
			pinMask = pin.mask[i];
		}
		pinMask = pinMask << currentBitOffset;
		oneBitCnt += bitCount(pinMask);
		currentBitOffset = 0;
	}
	float maxTransmittance = 1.0 - pin.maxColProb;
	float transmittance = pow(maxTransmittance, float(oneBitCnt));
	//SI_printf("Sampled transmittance: %f.3\n", transmittance);
	return transmittance;
};

#endif

#else
// Mock implementation, uses scalar field
float pinSampleDistance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	uint pinIdx = pin_cache_offset(origin,direction);
	vec3 newOrigin = pin_pos(pinIdx);
	vec3 newDirection = pin_dir(pinIdx);
	if(dot(newDirection, direction) < 0.0)
	{
		newDirection = -newDirection;
	}
	SI_setHistValue(dot(newDirection, direction), getFrameIdx())
	return rayMarcheMedium(newOrigin, newDirection, maxLength, seed);
};
float pinSampleTransmittance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	uint pinIdx = pin_cache_offset(origin,direction);
	vec3 newOrigin = pin_pos(pinIdx);
	vec3 newDirection = pin_dir(pinIdx);
	if(dot(newDirection, direction) < 0.0)
	{
		newDirection = -newDirection;
	}
	SI_setHistValue(dot(newDirection, direction), getFrameIdx())
	return rayMarcheMediumTransmittance(newOrigin, newDirection, maxLength, seed);
};
#endif


#endif
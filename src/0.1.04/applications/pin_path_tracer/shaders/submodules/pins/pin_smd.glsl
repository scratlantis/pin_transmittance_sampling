#ifndef PIN_SMD_H
#define PIN_SMD_H

#include "interface_structs.glsl"

layout(binding = PIN_SMD_BINDING_OFFSET) readonly buffer PIN_GRID
{
	GLSLPinCacheEntry pin_grid[];
};
#include "pin_common.glsl"
layout(constant_id = PIN_SMD_SPEC_CONST_OFFSET) const uint scBitMaskIterations = 5;

#ifndef DISABLE_BITMASK_DISTANCE_SAMPLING
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
		//finalMask = pinMask;

		uint sampledDiscreteDist = 31 - findMSB(finalMask);
		//SI_printf("Sampled distance: %d\n", currentBitOffset);
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
	//float sampledDist = float(intersectionBit + 0) * perBitDistance;
	//float sampledDist = float(intersectionBit - unormNext(seed)) * perBitDistance;
	
	if(sampledDist > maxLength)
	{
		return TMAX;
	}
	return sampledDist;
};
#else
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
#endif

#ifndef DISABLE_BITMASK_TRANSMITTANCE_SAMPLING
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

	float maxLengthCoef = sampleCoef + (maxLength / distance(start, end));
	uint maxLengthShift = uint(clamp(1.0 - maxLengthCoef, 0.0, 0.9999) * 32);
	uint maxLengthBitMask = 0xFFFFFFFF << maxLengthShift;

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
		pinMask &= maxLengthBitMask;

		pinMask = pinMask << currentBitOffset;
		oneBitCnt += bitCount(pinMask);
		currentBitOffset = 0;
	}
	float maxTransmittance = 1.0 - pin.maxColProb;
	float transmittance = pow(maxTransmittance, float(oneBitCnt));
	//SI_printf("Sampled transmittance: %f.3\n", transmittance);
	return transmittance;
};

#else
float pinSampleTransmittance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	uint pinIdx = pin_cache_offset(origin,direction);
	vec3 newOrigin = pin_pos(pinIdx);
	vec3 newDirection = pin_dir(pinIdx);
	bool inverte = dot(newDirection, direction) < 0.0;
	if(dot(newDirection, direction) < 0.0)
	{
		newDirection = -newDirection;
	}
	SI_setHistValue(dot(newDirection, direction), getFrameIdx())
	float transmittance1 = rayMarcheMediumTransmittance(newOrigin, newDirection, maxLength, seed);

	GLSLPinCacheEntry pin = pin_grid[pinIdx];
	float transmittance2;
	if(inverte)
	{
		transmittance2 = pin.transmittanceNeg;
	}
		else
	{
		transmittance2 = pin.transmittancePos;
	}
	return transmittance2;
};
#endif


#endif
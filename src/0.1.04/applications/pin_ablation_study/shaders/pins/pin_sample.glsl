#ifndef PIN_SAMPLE_H
#define PIN_SAMPLE_H

#include "interface_structs.glsl"
#include "pin_common.glsl"
float sampleMaskDistance(uint mask, vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	if(mask == 0)
	{
		return TMAX;
	}

	bool inverseDir = direction.z < 0;
	vec3 start, end;
	unitCubeIntersection(origin, direction, start, end);

	float relOffset = distance(origin, start) / distance(start, end); // [0, 1]
	relOffset = clamp(relOffset, 0.0, 0.9999); // [0, 1)
	uint bitOffset = uint(relOffset * 32); // [0, 31]
	float perBitDistance = distance(start, end) / float(32);

	if(inverseDir)
	{
		mask = bitfieldReverse(mask);
	}
	// x = position relative to pin
	// start		x			    end
	//      001010011011100111010110
	mask = mask << bitOffset; // Shift
	//      x			    end
	//      101110011101011000000000

	uint sampledDiscreteDist = 31 - findMSB(mask); // [0, 32] (32 == infinity)
	if(sampledDiscreteDist == 32)
	{
		return TMAX;
	}
	float sampledDist = float(sampledDiscreteDist) * perBitDistance;
	sampledDist += unormNext(seed) * perBitDistance; // randomize hit pos within the bit

	if(sampledDist > maxLength)
	{
		return TMAX;
	}
	return sampledDist;
}
uint sampleMaskHitCount(uint mask, vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	if(mask == 0)
	{
		return 0;
	}

	bool inverseDir = direction.z < 0;
	vec3 start, end;
	unitCubeIntersection(origin, direction, start, end);

	float perBitDistance = distance(start, end) / float(32);

	float relOffset = distance(origin, start) / distance(start, end); // [0, 1]
	relOffset = clamp(relOffset, 0.0, 0.9999); // [0, 1)
	uint bitOffset = uint(relOffset * 32); // [0, 31]

	float relEndOffset = relOffset + maxLength / distance(start, end); // [0, 1]
	float invRelEndOffset = 1.0 - relEndOffset;
	invRelEndOffset = clamp(invRelEndOffset, 0.0, 0.9999); // [0, 1)
	uint invBitEndOffset = uint(invRelEndOffset * 32); // [0, 31]

	uint maxLengthBitMask = 0xFFFFFFFF << invBitEndOffset;

	if(inverseDir)
	{
		mask = bitfieldReverse(mask);
	}
	// [ ] range over which we want to evaluate transmittance
	// start		[		]	    end
	//      001010011011100111010110
	mask &= maxLengthBitMask;
	// start		[		]	    end
	//      001010011011100110000000
	mask = mask << bitOffset; // Shift
	// 	    [		]	    end
	//      101110011000000000000000
	return bitCount(mask); // [0, 32]
}


// V1
float pinSampleDistanceV1(GLSLPinCacheEntryV1 pin, vec3 direction)
{
	bool invertDir = direction.z < 0;
	if(invertDir)
	{
		return pin.collisionDistance.x;
	}
	else
	{
		return pin.collisionDistance.y;
	}
}
float pinSampleTransmittanceV1(GLSLPinCacheEntryV1 pin, vec3 direction)
{
	bool invertDir = direction.z < 0;
	if(invertDir)
	{
		return pin.transmittance.x;
	}
	else
	{
		return pin.transmittance.y;
	}
}
// V2
float pinSampleDistanceV2(GLSLPinCacheEntryV2 pin, vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	return sampleMaskDistance(pin.mask, origin, direction, maxLength, seed);
}

float pinSampleTransmittanceV2(GLSLPinCacheEntryV2 pin, vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	return sampleMaskHitCount(pin.mask, origin, direction, maxLength, seed) > 0 ? 0.0 : 1.0;
}
// V3
float pinSampleDistanceV3(GLSLPinCacheEntryV3 pin, vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	float p = 1.0 - pin.minTransmittance;
	uint sampleMask = randomBitMask(p, RNG_SAMPLE_MASK_ITERATIONS, seed);
	uint mask = pin.mask & sampleMask;
	return sampleMaskDistance(mask, origin, direction, maxLength, seed);
}
float pinSampleTransmittanceV3(GLSLPinCacheEntryV3 pin, vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	uint hitCount = sampleMaskHitCount(pin.mask, origin, direction, maxLength, seed);
	float transmittance = pow(pin.minTransmittance, hitCount);
	return transmittance;
}

#if PIN_TYPE == 1
#define PIN_SAMPLE_DISTANCE(pin, origin, dir, maxLength, seed) pinSampleDistanceV1(pin, dir)
#define PIN_SAMPLE_TRANSMITTANCE(pin, origin, dir, maxLength, seed) pinSampleTransmittanceV1(pin, dir)
#elif PIN_TYPE == 2
#define PIN_SAMPLE_DISTANCE(pin, origin, dir, maxLength, seed) pinSampleDistanceV2(pin, origin, dir, maxLength, seed)
#define PIN_SAMPLE_TRANSMITTANCE(pin, origin, dir, maxLength, seed) pinSampleTransmittanceV2(pin, origin, dir, maxLength, seed)
#elif PIN_TYPE == 3
#define PIN_SAMPLE_DISTANCE(pin, origin, dir, maxLength, seed) pinSampleDistanceV3(pin, origin, dir, maxLength, seed)
#define PIN_SAMPLE_TRANSMITTANCE(pin, origin, dir, maxLength, seed) pinSampleTransmittanceV3(pin, origin, dir, maxLength, seed)
#endif

#endif



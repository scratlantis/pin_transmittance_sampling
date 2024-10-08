#ifndef PT_PIN_MEDIUM_H
#define PT_PIN_MEDIUM_H

#include "pt_params.glsl"
#include "../common.glsl"


// compile-time constants

#ifndef PIN_GRID_SIZE
#define PIN_GRID_SIZE 64
#endif

#ifndef PIN_COUNT_PER_GRID_CELL
#define PIN_COUNT_PER_GRID_CELL 4
#endif

#ifndef PIN_TRANSMITTANCE_VALUE_COUNT
#define PIN_TRANSMITTANCE_VALUE_COUNT 10
#endif

#ifndef RAY_MARCHE_STEP_SIZE
#define RAY_MARCHE_STEP_SIZE 0.2
#endif

//#define USE_PINS

#define PT_PIN_MEDIUM_BINDING_COUNT 2
layout(binding = PT_PIN_MEDIUM_BINDING_OFFSET) readonly buffer PIN_GRID {GLSLPinGridEntry pinGrid[];};
layout(binding = PT_PIN_MEDIUM_BINDING_OFFSET + 1) readonly buffer PIN_TRANSMITTANCE { float pinTransmittance[];};

#ifdef USE_PINS
GLSLPinGridEntry selectPin(vec3 origin, vec3 direction, float maxLenght, inout uint seed)
{
    origin += SCONST_PARAMS.jitterPinSamplePos * (1.0 - 2.0 * vec3(unormNext(seed), unormNext(seed), unormNext(seed)));
	origin = clamp(vec3(0.0),vec3(0.9999), origin);
	vec3 destination = origin + direction * maxLenght;
	destination = clamp(vec3(0.0),vec3(0.9999), destination);
	vec3 sampleLocation;
	if(SCONST_PARAMS.jitterPinSampleLocation != 0)
	{
		sampleLocation = mix(origin, destination, unormNext(seed));
	}
	else
	{
		sampleLocation = mix(origin, destination, SCONST_PARAMS.pinSampleLocation);
	}
	uvec3 gridId = uvec3(sampleLocation*float(PIN_GRID_SIZE));
	uint cellIdx = flatten(gridId, uvec3(PIN_GRID_SIZE)) * PIN_COUNT_PER_GRID_CELL;

	float maxDot = 0.0;
	GLSLPinGridEntry maxDotPin;
	for(uint i = 0; i < PIN_COUNT_PER_GRID_CELL; i++)
	{
		GLSLPinGridEntry gridEntry = pinGrid[cellIdx + i];
		float dotVal = abs(dot(getPinDirection(gridEntry.pin), direction));
		if(dotVal > maxDot)
		{
			maxDot = dotVal;
			maxDotPin = gridEntry;
		}
	}
	return maxDotPin;
}

float samplePin(vec3 origin, vec3 destination, GLSLPinGridEntry gridEntry, out vec2 segment)
{
	vec3 pinOrigin = getPinOrigin(gridEntry.pin);
	vec3 pinDestination = getPinDestination(gridEntry.pin);
	vec3 pinDirection = getPinDirection(gridEntry.pin);
	vec3 direction = normalize(destination - origin);

	segment = projectRaySegment(pinOrigin, pinDestination, origin, destination);

	vec2 sampleLocation = clamp(segment, 0.0, 0.9999) * vec2(PIN_TRANSMITTANCE_VALUE_COUNT);
	uvec2 rightSampleIndex = uvec2(sampleLocation);
	vec2 uv = sampleLocation - floor(sampleLocation);
	uint baseIndex = gridEntry.idx * PIN_TRANSMITTANCE_VALUE_COUNT;

	// right
	vec2 rightSampleValue = vec2(pinTransmittance[baseIndex + rightSampleIndex.x], pinTransmittance[baseIndex + rightSampleIndex.y]);

	// left
	vec2 leftSampleValue;
	if(rightSampleIndex.x == 0)
	{
		leftSampleValue.x = 0.0;
	}
	else
	{
		leftSampleValue.x = pinTransmittance[baseIndex + rightSampleIndex.x - 1];
	}
	if(rightSampleIndex.y == 0)
	{
		leftSampleValue.y = 0.0;
	}
	else
	{
		leftSampleValue.y = pinTransmittance[baseIndex + rightSampleIndex.y - 1];
	}

	vec2 sampleValue = mix(leftSampleValue, rightSampleValue, uv);
	float cumulativDensity = abs(sampleValue.x - sampleValue.y);

	float realDistance = distance(origin, destination);
	float pinDistance = distance(pinOrigin, pinDestination)*abs(segment.y - segment.x);
	
	float coef = 1.0;
	if(pinDistance > 0.01 && realDistance > 0.01)
	{
		coef = realDistance/pinDistance;
	}

	cumulativDensity *= coef;
	float transmittance = exp(-cumulativDensity);
	return transmittance;
}

float pinSampleDistanceMedium(vec3 origin, vec3 direction, float maxLenght, out vec3 pinStart, out vec3 pinEnd, inout uint seed)
{
	GLSLPinGridEntry pinGridEntry = selectPin(origin, direction, maxLenght, seed);
	vec3 pinOrigin = getPinOrigin(pinGridEntry.pin);
	vec3 pinDestination = getPinDestination(pinGridEntry.pin);
	vec3 pinDirection = getPinDirection(pinGridEntry.pin);
	vec3 destination = origin + direction * maxLenght;
	vec2 segment = projectRaySegment(pinOrigin, pinDestination, origin, destination);

	pinStart = mix(getPinOrigin(pinGridEntry.pin), getPinDestination(pinGridEntry.pin), segment.x);
	pinEnd = mix(getPinOrigin(pinGridEntry.pin), getPinDestination(pinGridEntry.pin), segment.y);

	bool sampleForward = segment.x < segment.y;
	uvec2 bitSegment = uvec2 (clamp(segment, 0.0, 0.9999) * vec2(32));
	uint sampleCount = uint(abs(bitSegment.x-bitSegment.y));
	//return 0.1;
	if(sampleCount == 0)
	{
		return TMAX;
	}
	float distancePerBit = maxLenght/float(sampleCount);
	float pdf = exp(-distancePerBit*pinGridEntry.maxDensity);
	uint sampleBitDistance;

	if(sampleForward)
	{
		uint sampleMask = 0;
		for(uint i = bitSegment.x; i<bitSegment.y;i++)
		{
			float rng = unormNext(seed);
			if(rng > pdf)
			{
				sampleMask |= 1 << 32-i;
				//sampleMask |= 1 << i;
			}
		}
		int sampleBitOffset = findMSB(sampleMask & pinGridEntry.densityMask);
		if(sampleBitOffset == -1)
		{
			return TMAX;
		}
		sampleBitDistance = sampleBitOffset - bitSegment.x;
	}
	else
	{
		uint sampleMask = 0;
		for(uint i = bitSegment.y; i<bitSegment.x;i++)
		{
			float rng = unormNext(seed);
			if(rng > pdf)
			{
				//sampleMask |= 1 << (32 - i);
				sampleMask |= 1 << i;
			}
		}
		int sampleBitOffset = findLSB(sampleMask & pinGridEntry.densityMask);
		if(sampleBitOffset == -1)
		{
			return TMAX;
		}
		sampleBitDistance = sampleBitOffset - bitSegment.y;
	}
	float sampleDistance =  sampleBitDistance*distancePerBit;
	


	return sampleDistance;
}


float pinMarcheMedium(vec3 origin, vec3 direction, float maxLenght, inout uint seed)
{
	float rng = unormNext(seed);
	const float max_steps = sqrt(3.0) * 2.0 / RAY_MARCHE_STEP_SIZE;
	float t = 0.0;
	float transmittance = 1.0;
	float stepSize = RAY_MARCHE_STEP_SIZE * unormNext(seed);
	stepSize = min(stepSize, maxLenght);
	vec2 uv;
	GLSLPinGridEntry pinGridEntry = selectPin(origin, direction, maxLenght, seed);

	for(uint i = 0; i < max_steps; i++)
	{
		t += stepSize;
		vec3 pos = origin + direction * (t-stepSize*unormNext(seed));

		// Sample transmittance
		transmittance = samplePin(origin, pos, pinGridEntry, uv);

		// Decide if we should stop
		if(transmittance < rng)
		{
			return t;
		}

		stepSize = min(RAY_MARCHE_STEP_SIZE, maxLenght - t);
		if( stepSize < EPSILON)
		{
			return TMAX;
		}
	}
	return TMAX;
}

float pinEvalTransmittance(vec3 origin, vec3 direction, float segmentLenght,
inout uint seed, out vec3 pinStart, out vec3 pinEnd, inout uint pinID)
{
	vec3 destination = origin + direction * segmentLenght;
	vec2 uv;
	GLSLPinGridEntry pinGridEntry = selectPin(origin, direction, segmentLenght, seed);
	pinID = pinGridEntry.idx;
	float transmittance = samplePin(origin, destination, pinGridEntry, uv);
	pinStart = mix(getPinOrigin(pinGridEntry.pin), getPinDestination(pinGridEntry.pin), uv.x);
	pinEnd = mix(getPinOrigin(pinGridEntry.pin), getPinDestination(pinGridEntry.pin), uv.y);
	return transmittance;
}
#endif


#endif
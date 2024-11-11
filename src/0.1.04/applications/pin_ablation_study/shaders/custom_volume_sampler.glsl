#ifndef CUSTOM_VOLUME_SAMPLER_H
#define CUSTOM_VOLUME_SAMPLER_H
#include "pins/pin_common.glsl

layout(constant_id = CUSTOM_VOLUME_SAMPLER_SPEC_CONST_OFFSET) const float jitterPos = 0.0;
layout(constant_id = CUSTOM_VOLUME_SAMPLER_SPEC_CONST_OFFSET + 1) const float jitterDir = 0.0;

// Sample type
// 0: No quantisation (Compute pin via ray marching)
// 1: Quantisation (Compute pin on demand via ray marching with quantised origin and direction)
// 2: Precomputed (Pin sampling)

// Pin type:
// 0: No pins
// 1-3: Pins V1-V3 (Precomputed pins)

#if PIN_TYPE == 0
float cvsSampleDistance(vec3 origin, vec3 dir, float maxLength, inout uint seed)
{
// quantisation
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
	quantise_to_pin_grid(origin, dir, maxLength, seed);
#endif
// fixed seed per grid cell (simulate pin usage)
#if(SAMPLE_TYPE == 2)
	uint tmpSeed = seed;
	seed = hash(pin_cache_offset(origin, dir)) ^ getSeed();
#endif
	float tLocal = rayMarcheMedium(origin, dir, maxLength, seed);
#if (SAMPLE_TYPE == 2)
	seed = tmpSeed;
#endif
	return tLocal;
}
float cvsSampleTransmittance(vec3 origin, vec3 dir, float maxLength, inout uint seed)
{
// quantisation
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
	quantise_to_pin_grid(origin, dir, maxLength, seed);
#endif
// fixed seed per grid cell (simulate pin usage)
#if(SAMPLE_TYPE == 2)
	uint tmpSeed = seed;
	seed = hash(pin_cache_offset(origin, dir)) ^ getSeed();
#endif
	float transmittance = rayMarcheMediumTransmittance(origin, dir, maxLength, seed);
#if (SAMPLE_TYPE == 2)
	seed = tmpSeed;
#endif
	return transmittance;
}

#else

// require pins
layout(binding = LOCAL_BINDING_OFFSET) readonly buffer PIN_GRID
{
	PIN_STRUCT pin_grid[];
};

float cvsSampleDistance(vec3 origin, vec3 dir, float maxLength, inout uint seed)
{
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
#endif
// quantisation
#if SAMPLE_TYPE == 1
	quantise_to_pin_grid(origin, dir, maxLength, seed);
#endif
// compute pin
#if SAMPLE_TYPE != 2
	PIN_STRUCT pin = CREATE_PIN(origin, dir, seed);
#else
	PIN_STRUCT pin = pin_grid[pin_cache_offset(origin, dir)];
#endif
	return PIN_SAMPLE_DISTANCE(pin, origin, dir, maxLength, seed);
}

float cvsSampleTransmittance(vec3 origin, vec3 dir, float maxLength, inout uint seed)
{
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
#endif
// quantisation
#if SAMPLE_TYPE == 1
	quantise_to_pin_grid(origin, dir, maxLength, seed);
#endif
// compute pin
#if SAMPLE_TYPE != 2
	PIN_STRUCT pin = CREATE_PIN(origin, dir, seed);
#else
	PIN_STRUCT pin = pin_grid[pin_cache_offset(origin, dir)];
#endif
	return PIN_SAMPLE_TRANSMITTANCE(pin, origin, dir, maxLength, seed);
}

#endif
#endif
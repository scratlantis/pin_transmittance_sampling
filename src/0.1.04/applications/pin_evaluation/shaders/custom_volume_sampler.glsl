#ifndef CUSTOM_VOLUME_SAMPLER_H
#define CUSTOM_VOLUME_SAMPLER_H
#include "pins/pin_create.glsl"
#include "pins/pin_sample.glsl"
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
float cvsSampleDistance(vec3 origin, vec3 dir, float maxLength, float selectionOffset, inout uint seed)
{
// quantisation
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
	quantise_to_pin_grid_with_jitter(origin, dir, seed);
	//quantise_to_pin_grid(origin, dir);
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
float cvsSampleTransmittance(vec3 origin, vec3 dir, float maxLength, float selectionOffset, inout uint seed)
{
// quantisation
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
	quantise_to_pin_grid_with_jitter(origin, dir, seed);
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
layout(binding = CUSTOM_VOLUME_SAMPLER_BINDING_OFFSET) readonly buffer PIN_GRID
{
	PIN_STRUCT pin_grid[];
};

float cvsSampleDistance(vec3 origin, vec3 dir, float maxLength, float selectionOffset, inout uint seed)
{
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
#endif
// quantisation
#if SAMPLE_TYPE == 1
	//quantise_to_pin_grid(origin, dir);
	//quantise_to_pin_grid_with_jitter(origin, dir, seed);
#endif
// compute pin
	vec3 selectionPos = origin + dir * selectionOffset * maxLength;
#if SAMPLE_TYPE != 2
	//if(dir.z < 0.0) dir = -dir;
	PIN_STRUCT pin = CREATE_PIN(selectionPos,(dir.z < 0.0) ? -dir :dir, seed);
#else
	PIN_STRUCT pin = pin_grid[pin_cache_offset(selectionPos, dir)];
#endif
	//pin = pin_grid[pin_cache_offset(selectionPos, dir)];
//SI_printf("PIN_TYPE %d\n",PIN_TYPE);
	return PIN_SAMPLE_DISTANCE(pin, origin, dir, maxLength, seed);
}

float cvsSampleTransmittance(vec3 origin, vec3 dir, float maxLength, float selectionOffset, inout uint seed)
{
#if SAMPLE_TYPE != 0
	apply_jitter(origin, dir, jitterPos, jitterDir, seed);
#endif
//apply_jitter(origin, dir, jitterPos, jitterDir, seed);
// quantisation
#if SAMPLE_TYPE == 1
	//quantise_to_pin_grid(origin, dir);
	//quantise_to_pin_grid_with_jitter(origin, dir, seed);
#endif
// compute pin
    vec3 selectionPos = origin + dir * selectionOffset * maxLength;
#if SAMPLE_TYPE != 2
	//PIN_STRUCT pin = CREATE_PIN(selectionPos, dir, seed);
	PIN_STRUCT pin = CREATE_PIN(selectionPos,(dir.z < 0.0) ? -dir :dir, seed);
#else
	PIN_STRUCT pin = pin_grid[pin_cache_offset(selectionPos, dir)];
#endif

	return PIN_SAMPLE_TRANSMITTANCE(pin, origin, dir, maxLength, seed);
}

#endif
#endif
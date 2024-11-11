#ifndef PIN_CREATE_H
#define PIN_CREATE_H

#include "pin_common.glsl"
GLSLPinCacheEntryV1 pin_create_v1(vec3 inOrigin, vec3 inDir, inout uint seed)
{
	vec3 origin = clamp(inOrigin, 0.0 , 0.99999);
	vec3 start, end;
	get_pin_segment(origin, inDir, start, end);
	vec3 dir = normalize(end - start);

	vec2 traceDistance = vec2(distance(origin, end), distance(origin, start));

	GLSLPinCacheEntryV1 pin;
	pin.transmittance.x = rayMarcheMediumTransmittance(origin, dir, traceDistance.x, seed); // +
	pin.transmittance.y = rayMarcheMediumTransmittance(origin, -dir, traceDistance.y, seed); // -

	pin.collisionDistance.x = rayMarcheMedium(origin, dir, traceDistance.x, seed); // +
	pin.collisionDistance.y = rayMarcheMedium(origin, -dir, traceDistance.y, seed); // -

	return pin;

}
GLSLPinCacheEntryV2 pin_create_v2(vec3 origin, vec3 dir, inout uint seed)
{
	vec3 origin = clamp(inOrigin, 0.0 , 0.99999);
	vec3 start, end;
	get_pin_segment(origin, inDir, start, end);
	vec3 dir = normalize(end - start);

	uint stepCount = 32;
	float stepLength = distance(end,start) / float(stepCount);
	uint mask = 0;
	for(uint i = 0; i<stepCount; i++)
	{
		vec3 pos = start + dir * float(i) * stepLength;
		float transmittance = rayMarcheMediumTransmittance(pos, dir, stepLength, seed);
		if(transmittance < unormNext(seed))
		{
			mask |= 1 << 31 - i;
		}
	}
	pin.mask = mask;
}
GLSLPinCacheEntryV3 pin_create_v3(vec3 origin, vec3 dir, inout uint seed)
{
	vec3 origin = clamp(inOrigin, 0.0 , 0.99999);
	vec3 start, end;
	get_pin_segment(origin, inDir, start, end);
	vec3 dir = normalize(end - start);

	uint stepCount = 32;
	float stepLength = distance(end,start) / float(stepCount);
	float minTransmittance = 1.0;
	for(uint i = 0; i<stepCount; i++)
	{
		vec3 pos = start + dir * float(i) * stepLength;
		float transmittance = rayMarcheMediumTransmittance(pos, dir, stepLength, seed); // Must set volume resolution, and step size
		minTransmittance = min(minTransmittance, transmittance);
	}
	pin.minTransmittance = minTransmittance;

	uint mask = 0;
	for(uint i = 0; i<stepCount; i++)
	{
		vec3 pos = start + dir * float(i) * stepLength;
		float transmittance = rayMarcheMediumTransmittance(pos, dir, stepLength, seed);
		float p = (1.0 - transmittance)/(1.0 - minTransmittance);

		if(p > unormNext(seed))
		{
			mask |= 1 << 31 - i;
		}
	}
	pin.mask = mask;
}


#if PIN_TYPE == 1
#define CREATE_PIN(origin, dir, seed) pin_create_v1(origin, dir, seed)
#elif PIN_TYPE == 2
#define CREATE_PIN(origin, dir, seed) pin_create_v2(origin, dir, seed)
#elif PIN_TYPE == 3
#define CREATE_PIN(origin, dir, seed) pin_create_v3(origin, dir, seed)
#endif



#endif
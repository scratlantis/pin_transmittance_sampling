#ifndef PIN_CREATE_H
#define PIN_CREATE_H


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



}
GLSLPinCacheEntryV3 pin_create_v3(vec3 origin, vec3 dir, inout uint seed);


#endif
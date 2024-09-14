
#ifndef PINS_COMMON_H
#define PINS_COMMON_H
#include "../interface.glsl"
#include "../math.glsl"



vec3 getPinOrigin(GLSLPin pin)
{
	return vec3(pin.ax_ay, pin.az_bx.x);
}

vec3 getPinDestination(GLSLPin pin)
{
	return vec3(pin.az_bx.y, pin.by_bz);
}

vec3 getPinDirection(GLSLPin pin)
{
	return normalize(getPinDestination(pin) - getPinOrigin(pin));
}

GLSLPin getPinFromOriginAndDirection(vec3 origin, vec3 direction)
{
	vec3 entry, exit;
	unitCubeIntersection(origin, direction, entry, exit);
	GLSLPin pin;
	pin.ax_ay = entry.xy;
	pin.az_bx = vec2(entry.z, exit.x);
	pin.by_bz = exit.yz;
	return pin;
}

#endif
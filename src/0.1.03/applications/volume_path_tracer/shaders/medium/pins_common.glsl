
#ifndef PINS_COMMON_H
#define PINS_COMMON_H
#include "../interface.glsl"
#include "../math.glsl"



vec3 pinOrigin(GLSLPin pin)
{
	return vec3(pin.ax_ay, pin.az_bx.x);
}

vec3 pinDestination(GLSLPin pin)
{
	return vec3(pin.az_bx.y, pin.by_bz);
}

vec3 pinDirection(GLSLPin pin)
{
	return normalize(pinDestination(pin) - pinOrigin(pin));
}

GLSLPin pinFromOriginAndDirection(vec3 origin, vec3 direction)
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
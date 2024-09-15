#ifndef COMMON_H
#define COMMON_H
#include "interface_structs.glsl"
#include "local_structs.glsl"
#include "vka_structs.glsl"

#include "math.glsl"
#include "random.glsl"
#include "sampling.glsl"

// Path tracing:
Ray genPrimaryRay(GLSLFrame frame, GLSLView view, uvec2 pixel, inout uint seed)
{
	Ray ray;
	const vec2 pixelCenter = vec2(gl_GlobalInvocationID.xy) + vec2(unormNext(seed), unormNext(seed));
	const vec2 pixelUV = pixelCenter / vec2(frame.width, frame.height);
	const vec2 d = pixelUV * 2.0 - 1.0;
	ray.origin = (view.invMat * vec4(0,0,0,1)).xyz;
	vec4 target = frame.invProjection * vec4(d.x, d.y, 1, 1) ;
	ray.direction = (view.invMat*vec4(normalize(target.xyz), 0)).xyz ;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.weight = vec3(1.0);
	return ray;
}

Ray reflectLampertDiffuse(MaterialData material, mat4x3 tangentFrame, inout uint seed)
{
	vec3 localDir = sampleCosineWeightedHemisphere(vec2(unormNext(seed), unormNext(seed)));
	Ray ray;
	ray.direction = normalize(tangentFrame * vec4(localDir,0.0));
	ray.origin = tangentFrame[3].xyz;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.weight = material.albedo;
	//ray.weight = (material.albedo / PI) * cos(theta)  /  (cos(theta) / PI);
	return ray;
}

vec3 lampertBRDF(vec3 Kd)
{
	return Kd / PI;
}

vec3 uniformScatterBSDF(vec3 Kd)
{
	return Kd / (4 * PI);
}

Ray scatterUniform(vec3 albedo, vec3 pos, inout uint seed)
{
	vec3 dir = sampleUniformSphere(vec2(unormNext(seed), unormNext(seed)));
	Ray ray;
	ray.direction = normalize(dir);
	ray.origin = pos;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.weight = albedo;
	//ray.weight = (albedo / (4 * PI)) / (1 / (4 * PI));
	return ray;
}


vec3 sampleAreaLight(vec3 pos, VKAAreaLight light, inout uint seed, out float pdf)
{
	vec2 xi = vec2(unormNext(seed), unormNext(seed));
	vec2 bary = sampleTriangleBarycentrics(xi);
	vec3 samplePos = bary.x * light.v0 + bary.y * light.v1 + (1.0 - bary.x - bary.y) * light.v2;
	vec3 sampleDir = normalize(samplePos - pos);
	float dist = distance(samplePos, pos);
	dist = clamp(dist, 0.05, 100.0);
	float dotSurfaceNormal = absDot(light.normal, -sampleDir);
	dotSurfaceNormal = clamp(dotSurfaceNormal, 0.05, 1.0);
	pdf = dist*dist / (triangleArea(light.v0, light.v1, light.v2) * dotSurfaceNormal);
	return samplePos;
}

// Pins:
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
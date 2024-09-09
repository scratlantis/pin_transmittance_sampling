
#include "../interface.glsl"
#include "../random.glsl"
#include "../vka_structs.glsl"
#include "pt_params.glsl"
#include "sampling.glsl"
#include "../math.glsl"

#ifndef PT_COMMON_H
#define PT_COMMON_H

struct Ray
{
	vec3 origin;
	vec3 direction;
	float tmin;
	float tmax;
};

struct MaterialData
{
	vec3 albedo;
	vec3 specular;
	vec3 emission;
	float roughness;
	float f0;
};

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
	return ray;
}

Ray reflectLampertDiffuse(MaterialData material, mat4x3 tangentFrame, inout uint seed, inout float pdf, inout vec3 weight)
{
	vec3 localDir = sampleCosineWeightedHemisphere(vec2(unormNext(seed), unormNext(seed)));
	// pdf = cos(theta) / PI
	// weight = albedo / PI * cos(theta)
	weight *= material.albedo;
	Ray ray;
	ray.direction = normalize(tangentFrame * vec4(localDir,0.0));
	ray.origin = tangentFrame[3].xyz;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
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

Ray scatterUniform(vec3 albedo, vec3 pos, inout uint seed, inout float pdf, inout vec3 weight)
{
	vec3 dir = sampleUniformSphere(vec2(unormNext(seed), unormNext(seed)));
	// pdf = 1 / (4 * PI)
	// weight = albedo / (4 * PI)
	weight *= albedo;
	Ray ray;
	ray.direction = normalize(dir);
	ray.origin = pos;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	return ray;
}


vec3 sampleAreaLight(mat4x3 tangentFrame, VKAAreaLight light, inout uint seed, out float localPdf)
{
	vec2 xi = vec2(unormNext(seed), unormNext(seed));
	vec2 bary = sampleTriangleBarycentrics(xi);
	vec3 samplePos = bary.x * light.v0 + bary.y * light.v1 + (1.0 - bary.x - bary.y) * light.v2;
	vec3 sampleDir = normalize(samplePos - tangentFrame[3]);
	localPdf = distanceSquared(samplePos,tangentFrame[3]) / (triangleArea(light.v0, light.v1, light.v2) * absDot(light.normal, -sampleDir));
	return samplePos;
}


#endif
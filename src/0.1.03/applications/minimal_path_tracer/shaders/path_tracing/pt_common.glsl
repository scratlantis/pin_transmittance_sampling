
#include "../interface.glsl"
#include "../random.glsl"
#include "../vka_structs.glsl"

#ifndef PT_COMMON_H
#define PT_COMMON_H

struct Ray
{
	vec3 origin;
	vec3 direction;
	float tmin;
	float tmax;
};

Ray genPrimaryRay(GLSLFrame frame, GLSLView view, uvec2 pixel)
{
	Ray ray;
	const vec2 pixelCenter = vec2(gl_GlobalInvocationID.xy) + vec2(0.5);
	const vec2 pixelUV = pixelCenter / vec2(frame.width, frame.height);
	const vec2 d = pixelUV * 2.0 - 1.0;
	ray.origin = (view.invMat * vec4(0,0,0,1)).xyz;
	vec4 target = frame.invProjection * vec4(d.x, d.y, 1, 1) ;
	ray.direction = (view.invMat*vec4(normalize(target.xyz), 0)).xyz ;
	ray.tmin = 0.001;
	ray.tmax = 10000.0;
	return ray;
}

#endif
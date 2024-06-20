#version 460
#extension GL_GOOGLE_include_directive : enable
#include "shaderStructs.glsl"
#include "random.glsl"
#include "brdf.glsl"
layout(location = 0) in vec3 fs_world_pos;
layout(location = 1) in vec3 fs_world_normal;
layout(location = 0) out vec4 outColor;

void main()
{
	uint seed = floatBitsToUint(random(gl_FragCoord.xy));

	outColor.a = 1.0;
	vec2 xi = {unormNext(seed), unormNext(seed)};
	vec3 L_tangentSpace = sampleLampert(xi);
	mat3 TNB = getTangentBase(fs_world_normal);
	outColor.rgb = (normalize(TNB*L_tangentSpace));
	outColor.rgb = vec3(0.0,0.0,1.0);
}
